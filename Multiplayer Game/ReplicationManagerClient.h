#pragma once

// TODO(you): World state replication lab session

class ReplicationManagerClient
{
public:
	void read(const InputMemoryStream& packet);

	void CreateGameObject(uint32 networkId, const InputMemoryStream& packet, GameObject* go);
};