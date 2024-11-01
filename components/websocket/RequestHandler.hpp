#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "esp_err.h"
#include "esp_http_server.h"
#include <memory>
#include <string>

class ResponseSender;
class EventManager;

class RequestHandler
{
public:
    RequestHandler(
        std::shared_ptr<ResponseSender> response_sender,
        std::shared_ptr<EventManager> event_manager);

    esp_err_t handle_request(httpd_req_t *req);

private:
    std::shared_ptr<ResponseSender> response_sender;
    std::shared_ptr<EventManager> event_manager;

    esp_err_t handle_handshake(httpd_req_t *req);
    esp_err_t process_message(httpd_req_t *req, httpd_ws_frame_t &ws_pkt, uint8_t *buf);
    esp_err_t receive_frame(httpd_req_t *req, httpd_ws_frame_t &ws_pkt, uint8_t *&buf);
    esp_err_t handle_heartbeat(httpd_req_t *req);
};

#endif // REQUEST_HANDLER_H
