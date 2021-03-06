// PlutoBridge.cpp: Definiert die exportierten Funktionen für die DLL-Anwendung.
//

#include "stdafx.h"


void OnSay(Entity* Entity, Team* team, char* message)
{
	if (!Network::Client.opened())
		return;

	sio::message::list args;

	char teambuffer[32];
	sprintf_s(teambuffer, "%i", Entity->Shared.Client->Session.Clientstate.CurrentTeam);

	args.push(Config::Key);
	args.push(Server::GetName().c_str());
	args.push(Entity->Shared.Client->Session.Clientstate.Name); //name
	args.push(teambuffer); //team
	args.push(message); //message

	Network::Client.socket()->emit("chat_message", args);
}

void OnDiscordMessage(sio::event &ev)
{
	std::string key = ev.get_messages()[0]->get_string();
	std::string serverName = ev.get_messages()[1]->get_string();
	std::string user = ev.get_messages()[2]->get_string();
	std::string msg = ev.get_messages()[3]->get_string();

	if (key != Config::Key)
	{
		CommandManager::Print("[PlutoBridge]::Keys do not match!\n");
		return;
	}

	if (serverName != Server::GetName().c_str())
		return;

	CommandManager::SayAll(("^1[D]^7" + user + ":" + msg).c_str());

}

void OnOpen()
{
	CommandManager::Print("[PlutoBridge]::Connection Opened!\n");
}

void OnFail()
{
	CommandManager::Print("[PlutoBridge]::Connection Failed!\n");
}

void OnClose(sio::client::close_reason reason)
{
	CommandManager::Print("[PlutoBridge]::Connection Closed!\n");
}

DWORD WINAPI InitThread(LPVOID arg)
{
	Sleep(2000); //waiting for the serverload
	CommandManager::Init();
	if (!Config::ReadConfig())
	{
		CommandManager::Print("[PlutoBridge]::Could not read the config.json!\n");
		CommandManager::Print("[PlutoBridge]::Aborting!\n");
		return 0;
	}

	CommandManager::Print("[PlutoBridge]::Loaded!\n");
	Network::Client.socket()->on("discord_message", &OnDiscordMessage);
	Network::Client.set_open_listener(&OnOpen);
	Network::Client.set_fail_listener(&OnFail);
	Network::Client.set_close_listener(&OnClose);

	Network::Client.connect(Config::Remote);
	return 0;
}


namespace PlutoBridge
{
	void Startup()
	{
		CreateThread(NULL, NULL, &InitThread, NULL, NULL, NULL);
		HookManager::InstallOnSay(OnSay);
	}
}
