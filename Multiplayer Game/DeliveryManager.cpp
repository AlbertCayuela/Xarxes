#include "Networks.h"
#include "DeliveryManager.h"

// TODO(you): Reliability on top of UDP lab session

Delivery::Delivery(uint32 sequenceNum) {

	this->sequenceNumber = sequenceNum;
	this->delegate = new DeliveryDelegate();
	this->dispatchTime = Time.time;
}

Delivery::~Delivery()
{
	if (delegate != nullptr) {
		delete delegate;
		delegate = nullptr;
	}
}

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
	packet << nextOutgoingSequenceNumber;

	Delivery* delivery = new Delivery(nextOutgoingSequenceNumber);

	/*delivery->sequenceNumber = nextOutgoingSequenceNumber++;
	delivery->dispatchTime = Time.time;*/

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
	return !pendingAcknowledges.empty();
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
	uint32 numOfAcks = pendingAcknowledges.size();

	packet << numOfAcks;

	for (uint32 sequenceNum : pendingAcknowledges) {
		packet << sequenceNum;

	}

	pendingAcknowledges.clear();
}

void DeliveryManager::processAckdSequenceNumbers(const InputMemoryStream& packet)
{
	uint32 numOfAcks;
	packet >> numOfAcks;


	for (int i = 0; i < numOfAcks; ++i)
	{

		uint32 sequenceNum;
		packet >> sequenceNum;

		//Look for the sequence numbers on the pending deliveries list
		for (std::list<Delivery*>::iterator del_it = pendingDeliveries.begin(); del_it != pendingDeliveries.end(); del_it++)
		{

			Delivery* del = *del_it;
			//If it exists, success! and delete it from the list
			if (del->sequenceNumber == sequenceNum)
			{

				if (del->delegate)
					del->delegate->OnDeliverySuccess(this);

				
				delete del;
				del = nullptr;

				pendingDeliveries.erase(del_it);

				break;
			}
		}
	}
}

void DeliveryManager::processTimedoutPackets()
{
	for (std::list<Delivery*>::iterator del_it = pendingDeliveries.begin(); del_it != pendingDeliveries.end();)
	{
		Delivery* del = *del_it;
		if (Time.time - del->dispatchTime >= PACKET_DELIVERY_TIMEOUT_SECONDS)
		{
			if (del->delegate)
				del->delegate->OnDeliveryFailure(this);

			delete del;
			del = nullptr;
			del_it = pendingDeliveries.erase(del_it);
		}
		else {
			del_it++;
		}
	}
}

void DeliveryManager::clear()
{
	for (Delivery* del : pendingDeliveries) {

		delete del;
	}

	pendingAcknowledges.clear();
	pendingDeliveries.clear();
	nextOutgoingSequenceNumber = 0;
	nextExpectedSequenceNumber = 0;
}

void DeliveryDelegate::OnDeliveryFailure(DeliveryManager* deliveryManager)
{
	GameObject* networkGOs[MAX_NETWORK_OBJECTS];
	uint16 count;

	App->modLinkingContext->getNetworkGameObjects(networkGOs, &count);

	for (int i = 0; i < count; i++)
	{
		NetworkUpdate(networkGOs[i]);
	}

}
