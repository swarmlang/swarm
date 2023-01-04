#ifndef SWARM_IGNITE_H
#define SWARM_IGNITE_H

#include <optional>
#include <ignite/thin/ignite_client.h>
#include <ignite/thin/ignite_client_configuration.h>
#include "../../shared/nslib.h"
#include "interfaces.h"
#include "../ISA.h"

using namespace ignite::thin;

namespace swarmc::Runtime {
    class VirtualMachine;
}

namespace swarmc::ISA {
    class ReferenceBinaryWalk;
    class BinaryReferenceWalk;
}

namespace swarmc::Runtime::Ignite {

    using KVString = cache::CacheClient<std::string, std::string>;
    using KVNumber = cache::CacheClient<std::string, int>;

    class Runtime : public nslib::IStringable {
    public:
        explicit Runtime(const std::string& prefix, const std::string& address) : _prefix(prefix), _address(address) {
            _config.SetEndPoints(address);
            _client = IgniteClient::Start(_config);

            _clusterStrings = getQualifiedCache("cluster::strings");

            auto name = prefix + "::cluster::numbers";
            _clusterNumbers = _client.GetOrCreateCache<std::string, int>(name.c_str());
        }

        transactions::ClientTransaction startClusterTx() {
            auto txs = _client.ClientTransactions();
            return txs.TxStart();
        }

        void asRetryableClusterTransaction(const std::function<void(transactions::ClientTransaction)>& cb) {
            std::size_t tries = 10;
            while (true) {
                tries -= 1;

                try {
                    auto tx = startClusterTx();
                    cb(tx);
                    tx.Commit();
                    return;
                } catch (ignite::IgniteError& e) {
                    if ( tries <= 0 ) {
                        assert(false);  // FIXME: this should be a swarm exception
                    }
                }
            }

            assert(false);  // this should be unreachable
        }

        [[nodiscard]] KVString getClusterStrings() const { return _clusterStrings; }

        [[nodiscard]] KVNumber getClusterNumbers() const { return _clusterNumbers; }

        [[nodiscard]] KVString getClusterQueue() {
            return getQualifiedCache("cluster::queue");
        }

        [[nodiscard]] KVString getQualifiedCache(const std::string& name) {
            auto cacheName = _prefix + "::" + name;
            return _client.GetOrCreateCache<std::string, std::string>(cacheName.c_str());
        }

    protected:
        std::string _prefix;
        std::string _address;
        IgniteClientConfiguration _config;
        IgniteClient _client;

        KVString _clusterStrings;
        KVNumber _clusterNumbers;
    };


    class GlobalServices : public IGlobalServices {
    public:
        explicit GlobalServices(Runtime* runtime) : _runtime(runtime) {}

        std::string getUuid() override {
            return nslib::uuid();
        }

        std::size_t getId() override {
            std::size_t id;
            _runtime->asRetryableClusterTransaction([this, &id](auto) {
                static const char* idKey = "GlobalServices::autoId";
                auto nums = _runtime->getClusterNumbers();

                if ( nums.ContainsKey(idKey) ) {
                    id = static_cast<std::size_t>(nums.Get(idKey));
                }

                nums.Put(idKey, static_cast<int>(id) + 1);
            });
            return id;
        }

        double random() override {
            return nslib::rand();
        }

        std::optional<std::string> getKeyValue(const std::string& key) override {
            std::optional<std::string> val = std::nullopt;
            auto qKeyStr = "kv::" + key;
            auto qKey = qKeyStr.c_str();
            _runtime->asRetryableClusterTransaction([this, &qKey, &val](auto) {
                auto strs = _runtime->getClusterStrings();
                if ( strs.ContainsKey(qKey) ) {
                    val = std::make_optional(strs.Get(qKey));
                }
            });
            return val;
        }

        void putKeyValue(const std::string& key, const std::string& value) override {
            auto qKeyStr = "kv::" + key;
            auto qKey = qKeyStr.c_str();
            _runtime->asRetryableClusterTransaction([this, &qKey, &value](auto) {
                auto strs = _runtime->getClusterStrings();
                strs.Put(qKey, value);
            });
        }

        void dropKeyValue(const std::string& key) override {
            auto qKeyStr = "kv::" + key;
            auto qKey = qKeyStr.c_str();
            _runtime->asRetryableClusterTransaction([this, &qKey](auto) {
                auto strs = _runtime->getClusterStrings();
                strs.Remove(qKey);
            });
        }

        std::string getNodeId() override {
            if ( _myNodeId.empty() ) {
                joinCluster();
            }

            return _myNodeId;
        }

        [[nodiscard]] std::string toString() const override {
            return "Ignite::GlobalServices<myNodeId: " + _myNodeId + ">";
        }

    protected:
        Runtime* _runtime;
        std::string _myNodeId;

        void joinCluster() {
            std::string id = getUuid();
            std::string idKeyString = "nodes::by_id::" + id;
            const char* idKey = idKeyString.c_str();

            bool joined = false;
            std::size_t idx = 0;
            while ( !joined ) {
                _runtime->asRetryableClusterTransaction([&idKey, &id, &idx, &joined, this](auto) {
                    auto strs = _runtime->getClusterStrings();
                    auto nums = _runtime->getClusterNumbers();
                    std::string idxKeyString = "nodes::by_idx::" + s(idx);
                    const char* idxKey = idxKeyString.c_str();
                    idx += 1;

                    if ( !strs.ContainsKey(idxKey) ) {
                        strs.Put(idxKey, id);
                        nums.Put(idKey, static_cast<int>(idx));
                        joined = true;
                    }
                });
            }

            _myNodeId = id;
        }
    };


    class StorageInterface : public IStorageInterface {
    public:
        StorageInterface(Runtime* runtime, ISA::Affinity affinity);

        ISA::Reference* load(ISA::LocationReference* loc) override;

        void store(ISA::LocationReference* loc, ISA::Reference* value) override;

        bool has(ISA::LocationReference* loc) override;

        bool manages(ISA::LocationReference* loc) override;

        void drop(ISA::LocationReference* loc) override;

        const Type::Type* typeOf(ISA::LocationReference* loc) override;

        void typify(ISA::LocationReference* loc, const Type::Type* type) override;

        IStorageLock* acquire(ISA::LocationReference* loc) override;

        void clear() override;

        IStorageInterface* copy() override;

        [[nodiscard]] std::string toString() const override {
            return "Ignite::StorageInterface<a: " + s(_affinity) + ">";
        }

    protected:
        Runtime* _runtime;
        ISA::Affinity _affinity;
        std::string _uuid;
        KVString _cache;
        ISA::ReferenceBinaryWalk* _ref;
        ISA::BinaryReferenceWalk* _bin;
    };


    class StorageLock : public IStorageLock {
    public:
        StorageLock(StorageInterface* store, ISA::LocationReference* loc) {
            _loc = loc;
            _store = store;
        }

        [[nodiscard]] ISA::LocationReference* location() const override;

        void release() override;

        [[nodiscard]] std::string toString() const override;
    protected:
        ISA::LocationReference* _loc = nullptr;
        StorageInterface* _store = nullptr;
    };


    class QueueJob : public IQueueJob {
    public:
        QueueJob(Runtime* runtime, JobID id, IFunctionCall* call, const ScopeFrame* scope, const State* vmState):
            _runtime(runtime), _id(id), _call(call), _scope(scope), _vmState(vmState) {}

        [[nodiscard]] JobID id() const override { return _id; }

        [[nodiscard]] JobState state() const override;

        [[nodiscard]] IFunctionCall* getCall() const override { return _call; }

        [[nodiscard]] const ScopeFrame* getScope() const override { return _scope; }

        [[nodiscard]] const State* getState() const override { return _vmState; }

        [[nodiscard]] std::string toString() const override;

        void setFilters(SchedulingFilters filters) override { _filters = std::move(filters); }

        [[nodiscard]] SchedulingFilters getFilters() const override { return _filters; }

    protected:
        Runtime* _runtime;
        JobID _id;
        IFunctionCall* _call;
        SchedulingFilters _filters;
        const ScopeFrame* _scope;
        const State* _vmState;
    };


    class Queue : public IQueue {
    public:
        explicit Queue(Runtime* runtime, VirtualMachine* vm);

        ~Queue() override;

        void setContext(QueueContextID ctx) override {
            _context = ctx;
        }

        QueueContextID getContext() override { return _context; }

        bool shouldHandle(IFunctionCall* call) override { return true; }

        QueueJob* build(IFunctionCall* call, const ScopeFrame* scope, const State* state) override;

        void push(IQueueJob* job) override;

        IQueueJob* pop() override;

        bool isEmpty() override;

        [[nodiscard]] std::string toString() const override {
            return "Ignite::Queue<ctx: " + _context + ">";
        }

    protected:
        Runtime* _runtime;
        VirtualMachine* _vm;
        QueueContextID _context;
        ISA::BinaryReferenceWalk* _bin;
        ISA::ReferenceBinaryWalk* _ref;

        binn* serializeCall(IFunctionCall* call);

        binn* serializeScope(const ScopeFrame* scope);
    };

}

#endif //SWARM_IGNITE_H
