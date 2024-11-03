#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "../../managed_components/johboh__nlohmann-json/single_include/nlohmann/json.hpp"
#include "EventManager.hpp"
#include "ResponseSender.hpp"
#include "Utilities.hpp"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include <cstring>
#include <memory>
#include <string>

class ResponseSender;
class EventManager;

class RequestHandler
{
public:
    RequestHandler(
        std::shared_ptr<ResponseSender> response_sender,
        std::shared_ptr<EventManager> event_manager,
        std::shared_ptr<ClientManager> client_manager);

    esp_err_t handle_request(httpd_req_t *req);
    // esp_err_t handle_request(httpd_req_t *req);

private:
    std::shared_ptr<ResponseSender> response_sender;
    std::shared_ptr<EventManager> event_manager;
    std::shared_ptr<ClientManager> client_manager;

    esp_err_t handle_handshake(httpd_req_t *req);
    esp_err_t validate_jsonrpc2_header(httpd_req_t *req);
    // esp_err_t validate_jsonrpc2_header(httpd_req_t *req);
    esp_err_t process_message(httpd_req_t *req, httpd_ws_frame_t &ws_pkt, uint8_t *buf, std::string client_id);
    esp_err_t handle_heartbeat(httpd_req_t *req);
    esp_err_t identify_client(httpd_req_t *req, std::string &client_id);
};

#endif // REQUEST_HANDLER_H
