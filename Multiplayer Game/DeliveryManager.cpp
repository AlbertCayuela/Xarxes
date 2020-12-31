#include "Networks.h"
#include "DeliveryManager.h"

Delivery::~Delivery()
{
	if (delegate != nullptr) 
	{
		delete delegate;
		delegate = nullptr;
	}
}

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
	packet << nextOutSequenceNumber;

	Delivery* delivery = new Delivery();
	delivery->sequenceNumber = nextOutSequenceNumber;
	delivery->delegate = new DeliveryDelegate();
	delivery->dispatchTime = 0.0f;

	pendingDeliveries.push_back(delivery);

	nextOutSequenceNumber++;

	return delivery;
}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
	bool ret = false;
	uint32 sequenceNum = 0;
	packet >> sequenceNum;

	if (sequenceNum == nextInSequenceNumber) 
	{
		ret = true;
		nextInSequenceNumber++;
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
	uint32 acknowledgesNumber = pendingAcknowledges.size();
	packet << acknowledgesNumber;
	for (uint32 sequenceNum : pendingAcknowledges)
	{
		packet << sequenceNum;
	}
	pendingAcknowledges.clear();
}

void DeliveryManager::processAckdSequenceNumbers(const InputMemoryStream& packet)
{

	uint32 acknowledgesNumber;
	packet >> acknowledgesNumber;

	for (int i = 0; i < acknowledgesNumber; ++i)
	{
		uint32 sequenceNum;
		packet >> sequenceNum;
		for (std::list<Delivery*>::iterator i = pendingDeliveries.begin(); i != pendingDeliveries.end(); i++)
		{
			Delivery* delivery = (*i);
			if (delivery->sequenceNumber == sequenceNum)
			{
				delete delivery;
				delivery = nullptr;
				pendingDeliveries.erase(i);
				break;
			}
		}
	}
}

void DeliveryManager::processTimedOutPackets()
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
	nextOutSequenceNumber = 0;
	nextInSequenceNumber = 0;
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
