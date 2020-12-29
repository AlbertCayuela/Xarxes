#include "Networks.h"
#include "DeliveryManager.h"

// TODO(you): Reliability on top of UDP lab session

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
	packet << nextOutgoingSequenceNumber;

	Delivery* delivery = new Delivery();

	delivery->sequenceNumber = nextOutgoingSequenceNumber++;
	delivery->dispatchTime = Time.time;

	pendingDeliveries.push_back(delivery);

	return delivery;
}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{

	bool ret = false;

	uint32 sequenceNum = 0;
	packet >> sequenceNum;

	if (sequenceNum == nextExpectedSequenceNumber)
	{
		ret = true;
		nextExpectedSequenceNumber++;
	}

	pendingAcknowledges.push_back(sequenceNum);

	return ret;
}

bool DeliveryManager::hasSequenceNumbersPendingAck() const
{
	return false;
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
}

void DeliveryManager::processAckdSequenceNumbers(const InputMemoryStream& packet)
{
}

void DeliveryManager::processTimedoutPackets()
{
}

void DeliveryManager::clear()
{
}
