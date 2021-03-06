#include "Networks.h"
#include "ReplicationManagerClient.h"

// TODO(you): World state replication lab session

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
	while (packet.RemainingByteCount() > sizeof(uint32)) //Input last sequence number
	{
		uint32 networkId;
		ReplicationAction repAction;

		packet >> networkId;
		packet >> repAction;

		switch (repAction) {
		case ReplicationAction::Create:
		{
			GameObject* go = Instantiate();
			if (go)
			{
				CreateGameObject(networkId, packet, go);
				App->modLinkingContext->registerNetworkGameObjectWithNetworkId(go, networkId);
			}
			break;
		}

		case ReplicationAction::Update:
		{
			vec2 position;
			packet >> position.x;
			packet >> position.y;

			float angle;
			packet >> angle;

			vec2 size;
			packet >> size.x;
			packet >> size.y;

			uint8 type;
			packet >> type;

			GameObject* go = App->modLinkingContext->getNetworkGameObject(networkId);

			if (go) 
			{
				go->position = position;
				go->angle = angle;
			}
			//TODO: WHAT IF GAMEOBJECT DOESNT EXIST?
			else
			{
				go = Instantiate();

				go->position = position;
				go->angle = angle;
				go->size = size;

				//Create sprite
				go->sprite = App->modRender->addSprite(go);
				go->sprite->order = 5;

				//Put texture

				if (type == 0) {
					go->sprite->texture = App->modResources->laser;

				}
				else if (type == 1) {
					go->sprite->texture = App->modResources->spacecraft1;

				}
				else if (type == 2) {
					go->sprite->texture = App->modResources->spacecraft2;

				}
				else if (type == 3) {
					go->sprite->texture = App->modResources->spacecraft3;
				}
				

				App->modLinkingContext->registerNetworkGameObjectWithNetworkId(go, networkId);
			}

			break;
		}

		case ReplicationAction::Destroy:
		{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(networkId);

			if (go) {

				App->modLinkingContext->unregisterNetworkGameObject(go);
				Destroy(go);
			}

			break;
		}
		
		case ReplicationAction::None:
			//Nothing
			break;
		}
	}
}

void ReplicationManagerClient::CreateGameObject(uint32 networkId, const InputMemoryStream& packet, GameObject* go)
{
	//TODO: Fill
	uint8 type;

	packet >> go->position.x;
	packet >> go->position.y;
	packet >> go->angle;
	packet >> go->size.x;
	packet >> go->size.y;
	packet >> type;

	go->sprite = App->modRender->addSprite(go);
	go->sprite->order = 5;

	if (type == 0)
	{
		go->sprite->texture = App->modResources->laser;
	}
	else if (type == 1)
	{
		go->sprite->texture = App->modResources->spacecraft1;
	}
	else if (type == 2)
	{
		go->sprite->texture = App->modResources->spacecraft2;
	}
	else if (type == 3)
	{
		go->sprite->texture = App->modResources->spacecraft3;
	}

	if (type != 0)
	{
		go->collider = App->modCollision->addCollider(ColliderType::Player, go);
		go->collider->isTrigger = true;
		go->behaviour = new Spaceship;
		go->behaviour->gameObject = go;
	}
	else
	{
		go->collider = App->modCollision->addCollider(ColliderType::Laser, go);
		go->collider->isTrigger = true;
		go->sprite->order = 1;
	}
}
