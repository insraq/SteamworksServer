#include <iostream>
#include <external/steam/steam_api.h>
#include <external/httplib.h>
#include <external/json.hpp>

using json = nlohmann::json;
using namespace httplib;

static const char* json_type = "application/json";

int main(int argc, char* argv[])
{
	setvbuf(stdout, NULL, _IONBF, 0);

	if (!SteamAPI_Init())
	{
		printf("Steam must be running to play this game (SteamAPI_Init() failed)");
		return 1;
	}

	Server server;

	server.Get("/SteamApps/GetDLCCount", [](const Request&, Response& res) {
		json j = { {"result", SteamApps()->GetDLCCount()} };
		res.set_content(j.dump(), json_type);
		});

	server.Get("/SteamApps/GetCurrentBetaName", [](const Request&, Response& res) {
		char name[64];
		SteamApps()->GetCurrentBetaName(name, sizeof(name));
		json j = { {"result", name} };
		res.set_content(j.dump(), json_type);
		});

	server.Get("/SteamApps/MarkContentCorrupt", [](const Request& req, Response& res) {
		json j = { {"result", SteamApps()->MarkContentCorrupt(req.has_param("bMissingFilesOnly"))} };
		res.set_content(j.dump(), json_type);
		});

	server.Post("/SteamUser/GetSteamID", [](const Request& req, Response& res) {
		json j = { {"result", SteamUser()->GetSteamID().ConvertToUint64()}};
		res.set_content(j.dump(), json_type);
		});
	
	server.set_pre_routing_handler([](const auto& req, auto& res) {
		if (req.method == "OPTIONS") {
			res.status = 204;
			return Server::HandlerResponse::Handled;
		}
		return Server::HandlerResponse::Unhandled;
		});

	server.set_post_routing_handler([](const auto& req, auto& res) {
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_header("Access-Control-Allow-Method", "GET,HEAD,PUT,PATCH,POST,DELETE");
		res.set_header("Access-Control-Allow-Headers", req.get_header_value("Access-Control-Request-Headers"));
		});

	int port = argc == 2 ? atoi(argv[1]) : 9000;
	printf("Starting Steamworks Server on port %d", port);
	server.listen("localhost", port);

	return 0;
}