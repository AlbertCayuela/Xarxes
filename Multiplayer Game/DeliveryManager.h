#pragma once
#include <list>

class DeliveryManager;

// TODO(you): Reliability on top of UDP lab session
class DeliveryDelegate 
{
public:

	virtual void OnDeliverySuccess(DeliveryManager* deliveryManager) = 0;
	virtual void OnDeliveryFailure(DeliveryManager* deliveryManager) = 0;
};

struct Delivery 
{
	uint32 sequenceNumber = 0;
	double dispatchTime = 0.0f;
	DeliveryDelegate* delegate = nullptr;
};

class DeliveryManager
{
public:

	Delivery* writeSequenceNumber(OutputMemoryStream& packet);

	bool processSequenceNumber(const InputMemoryStream& packet);

	bool hasSequenceNumbersPendingAck() const;
	void writeSequenceNumbersPendingAck(OutputMemoryStream& packet);

	void processAckdSequenceNumbers(const InputMemoryStream& packet);
	void processTimedoutPackets();

	void clear();

private:

	uint32 nextOutgoingSequenceNumber = 0u;
	std::list<Delivery*> pendingDeliveries;

	uint32 nextIncomningSeqNumber = 0u;
	std::list<uint32> pendingAcknowledges;
};