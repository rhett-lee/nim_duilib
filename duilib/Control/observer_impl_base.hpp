#ifndef OBSERVER_IMPL_BASE_HPP
#define OBSERVER_IMPL_BASE_HPP

template <typename ReturnT, typename ParamT>
class ReceiverImplBase;

template <typename ReturnT, typename ParamT>
class ObserverImplBase
{
public:
    ObserverImplBase() = default;
    virtual ~ObserverImplBase() = default;
    virtual void AddReceiver(ReceiverImplBase<ReturnT, ParamT>* receiver) = 0;
    virtual void RemoveReceiver(ReceiverImplBase<ReturnT, ParamT>* receiver) = 0;
    virtual ReturnT Broadcast(ParamT param) = 0;
    virtual ReturnT RBroadcast(ParamT param) = 0;
    virtual ReturnT Notify(ParamT param) = 0;
};

template <typename ReturnT, typename ParamT>
class ReceiverImplBase
{
public:
    ReceiverImplBase() = default;
    virtual ~ReceiverImplBase() = default;
    virtual void AddObserver(ObserverImplBase<ReturnT, ParamT>* observer) = 0;
    virtual void RemoveObserver() = 0;
    virtual ReturnT Receive(ParamT param) = 0;
    virtual ReturnT Respond(ParamT param, ObserverImplBase<ReturnT, ParamT>* observer) = 0;
};

template <typename ReturnT, typename ParamT>
class ReceiverImpl;

template <typename ReturnT, typename ParamT>
class ObserverImpl : public ObserverImplBase<ReturnT, ParamT>
{
    //template <typename ReturnT, typename ParamT>
    friend class Iterator;
public:
    ObserverImpl() = default;
    virtual ~ObserverImpl() override = default;

    virtual void AddReceiver(ReceiverImplBase<ReturnT, ParamT>* receiver) override
    {
        if (receiver == nullptr) {
            return;
        }
        if (std::find(receivers_.cbegin(), receivers_.cend(), receiver) != receivers_.cend()) {
            ASSERT(0);
            return;
        }

        receivers_.push_back(receiver);
        receiver->AddObserver(this);
    }

    virtual void RemoveReceiver(ReceiverImplBase<ReturnT, ParamT>* receiver) override
    {
        if (receiver == nullptr) {
            return;
        }

        auto it = receivers_.begin();
        for (; it != receivers_.end(); ++it) {
            if (*it == receiver) {
                receivers_.erase(it);
                break;
            }
        }
    }

    virtual ReturnT Broadcast(ParamT param) override
    {
        auto it = receivers_.begin();
        for (; it != receivers_.end(); ++it) {
            (*it)->Receive(param);
        }

        return ReturnT();
    }

    virtual ReturnT RBroadcast(ParamT param) override
    {
        auto it = receivers_.rbegin();
        for (; it != receivers_.rend(); ++it) {
            (*it)->Receive(param);
        }

        return ReturnT();
    }

    virtual ReturnT Notify(ParamT param) override
    {
        auto it = receivers_.begin();
        for (; it != receivers_.end(); ++it) {
            (*it)->Respond(param, this);
        }

        return ReturnT();
    }

    template <typename ReturnType, typename ParamType>
    class Iterator
    {
        ObserverImpl<ReturnType, ParamType>& _tbl;
        uint32_t index;
        ReceiverImplBase<ReturnType, ParamType>* ptr;
    public:
        explicit Iterator(ObserverImpl& table)
            : _tbl(table), index(0), ptr(nullptr)
        {}

        explicit Iterator(const Iterator& v)
            : _tbl(v._tbl), index(v.index), ptr(v.ptr)
        {}

        ReceiverImplBase<ReturnType, ParamType>* next()
        {
            if (index >= _tbl.receivers_.size()) {
                return nullptr;
            }

            for (; index < _tbl.receivers_.size(); ) {
                ptr = _tbl.receivers_[index++];
                if (ptr) {
                    return ptr;
                }
            }
            return nullptr;
        }
    };

protected:
    typedef std::vector<ReceiverImplBase<ReturnT, ParamT>*> ReceiversVector;
    ReceiversVector receivers_;
};


template <typename ReturnT, typename ParamT>
class ReceiverImpl : public ReceiverImplBase<ReturnT, ParamT>
{
public:
    ReceiverImpl() = default;
    virtual ~ReceiverImpl() override = default;

    virtual void AddObserver(ObserverImplBase<ReturnT, ParamT>* observer) override
    {
        observers_.push_back(observer);
    }

    virtual void RemoveObserver() override
    {
        auto it = observers_.begin();
        for (; it != observers_.end(); ++it) {
            (*it)->RemoveReceiver(this);
        }
    }

    virtual ReturnT Receive(ParamT /*param*/) override
    {
        return ReturnT();
    }

    virtual ReturnT Respond(ParamT /*param*/, ObserverImplBase<ReturnT, ParamT>* /*observer*/) override
    {
        return ReturnT();
    }

protected:
    typedef std::vector<ObserverImplBase<ReturnT, ParamT>*> ObserversVector;
    ObserversVector observers_;
};

#endif // OBSERVER_IMPL_BASE_HPP
