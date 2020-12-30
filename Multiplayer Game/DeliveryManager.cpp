#include "Networks.h"
#include "DeliveryManager.h"

// TODO(you): Reliability on top of UDP lab session

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

	Delivery* delivery = new Delivery();

	delivery->sequenceNumber = nextOutgoingSequenceNumber++;
	delivery->delegate = new DeliveryDelegate();
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
	return !pendingAcknowledges.empty();
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
	uint32 numOfAcks = pendingAcknowledges.size();

	packet << numOfAcks;

	for (uint32 sequenceNum : pendingAcknowledges) 
	{
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

		for (std::list<Delivery*>::iterator i = pendingDeliveries.begin(); i != pendingDeliveries.end(); i++)
		{
			Delivery* del = *i;
			if (del->sequenceNumber == sequenceNum)
			{
				delete del;
				del = nullptr;

				pendingDeliveries.erase(i);

				break;
			}
		}
	}
}

void DeliveryManager::processTimedoutPackets()
{
	for (std::list<Delivery*>::iterator i = pendingDeliveries.begin(); i != pendingDeliveries.end();)
	{
		Delivery* delivery = *i;
		if (Time.time - delivery->dispatchTime >= PACKET_DELIVERY_TIMEOUT_SECONDS)
		{
			if (delivery->delegate) 
			{
				delivery->delegate->OnDeliveryFailure(this);
			}
			
			delete delivery;
			delivery = nullptr;
			i = pendingDeliveries.erase(i);
		}
		else 
		{
			i++;
		}
	}
}

void DeliveryManager::clear()
{
	for (Delivery* delivery : pendingDeliveries)
	{
		delete delivery;
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
