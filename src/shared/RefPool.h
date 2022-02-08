#ifndef SWARMC_REFPOOL_H
#define SWARMC_REFPOOL_H

#include <list>

template <typename T>
class Ref;

template <typename T>
class RefPool;

template <typename T>
class RefInstance {
public:
    RefInstance(T* item, Ref<T>* ref) : _item(item), _ref(ref) {};
    virtual ~RefInstance();
    RefInstance<T>* newInstance();

    T* get() {
        return _item;
    }
private:
    T* _item;
    Ref<T>* _ref;
};

template <typename T>
class Ref {
    friend class RefInstance<T>;

public:
    Ref(T* item, RefPool<T>* pool) : _item(item), _pool(pool) {};

    virtual ~Ref() {
        delete _item;
    };

    RefInstance<T>* get() {
        RefInstance<T>* inst = new RefInstance<T>(_item, this);
        instances += 1;
        return inst;
    }
private:
    T* _item = nullptr;
    RefPool<T>* _pool = nullptr;
    int instances = 0;
    void release();
};

template <typename T>
class RefPool {
    friend class Ref<T>;

public:
    static RefPool* get();

    RefPool() {
        refs = new std::list<Ref<T>*>();
    }

    ~RefPool() {
        for ( auto ref : *refs ) {
            delete ref;
        }

        delete refs;
    }

    Ref<T>* alloc(T* item) {
        Ref<T>* ref = new Ref<T>(item, this);
        refs->push_back(ref);
        return ref;
    }

    RefInstance<T>* allocInstance(T* item) {
        Ref<T>* ref = this->alloc(item);
        return ref->get();
    }
private:
    static RefPool<T>* _global;
    std::list<Ref<T>*>* refs;
    void tryfree(Ref<T>* ref) {
        refs->remove(ref);
        delete ref;
    }
};

#include "RefPool.icpp"

#endif
