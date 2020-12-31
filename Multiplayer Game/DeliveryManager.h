#pragma once
#include <list> 

class DeliveryManager;
class DeliveryDelegate
{
public:
	void OnDeliveryFailure(DeliveryManager* deliveryManager);
};

struct Delivery
{
	~Delivery();

	uint32 sequenceNumber = 0u;
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
	void processTimedOutPackets();
	void clear();

private:

	uint32 nextOutSequenceNumber = 0u;
	std::list<Delivery*> pendingDeliveries;

	uint32 nextInSequenceNumber = 0u;
	std::list<uint32> pendingAcknowledges;

};