#ifndef SWARMVM_LOCALSTREAM
#define SWARMVM_LOCALSTREAM

#include "../../shared/nslib.h"
#include "interfaces.h"

using namespace nslib;

namespace swarmc::Runtime {

        class LocalStream : public IStream, public IUsesConsole {
        public:
            explicit LocalStream(std::string id) : IUsesConsole(), _id(std::move(id)) {}

            void open() override {}

            void close() override {}

            bool isOpen() override { return true; }

            const Type::Type* innerType() override;

            ISA::Reference* pop() override { return nullptr; }

            bool isEmpty() override { return true; }

            [[nodiscard]] std::string id() const override { return _id; }

        protected:
            std::string _id;
        };


        class LocalOutputStream : public LocalStream {
        public:
            LocalOutputStream() : LocalStream("l:STDOUT") {}

            void push(ISA::Reference* value) override;

            [[nodiscard]] std::string toString() const override {
                return "LocalOutputStream<>";
            }
        };


        class LocalErrorStream : public LocalStream {
        public:
            LocalErrorStream() : LocalStream("l:STDERR") {}

            void push(ISA::Reference* value) override;

            [[nodiscard]] std::string toString() const override {
                return "LocalErrorString<>";
            }
        };

}

#endif //SWARMVM_LOCALSTREAM
