#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include "esp_check.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"

#include "TypeDefs.hpp"
#include "events.hpp"
#include "utils.hpp"

#include <cstdlib>
#include <cstring>

class WebSocket
{
public:
    WebSocket(httpd_handle_t server, esp_event_loop_handle_t system_event_loop);
    ~WebSocket();

    /**
     * @brief Start the WebSocket server.
     *
     * @return esp_err_t ESP_OK on success, otherwise an error code.
     */
    esp_err_t start();

    /**
     * @brief Stop the WebSocket server.
     *
     * @return esp_err_t ESP_OK on success, otherwise an error code.
     */
    esp_err_t stop();

    /**
     * @brief Disconnect event handler.
     *
     * @param arg Pointer to WebSocket instance.
     * @param event_base Event base.
     * @param event_id Event ID.
     * @param event_data Event data.
     */
    static void disconnect_handler(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data);

    /**
     * @brief Connect event handler.
     *
     * @param arg Pointer to WebSocket instance.
     * @param event_base Event base.
     * @param event_id Event ID.
     * @param event_data Event data.
     */
    static void connect_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data);

private:
    static constexpr const char *TAG = "WebSocket";
    httpd_handle_t server;
    esp_event_loop_handle_t system_event_loop;

    /**
     * @brief Structure holding server handle and internal socket fd for async operations.
     */
    struct AsyncRespArg
    {
        httpd_handle_t hd;
        int fd;
    };

    /**
     * @brief Asynchronous send function to be queued in the HTTPD work queue.
     *
     * @param arg Pointer to AsyncRespArg structure.
     */
    static void ws_async_send(void *arg);

    /**
     * @brief Trigger an asynchronous send.
     *
     * @param handle HTTP server handle.
     * @param req HTTP request.
     * @return esp_err_t ESP_OK on success, otherwise an error code.
     */
    esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req);

    /**
     * @brief Echo handler for WebSocket messages.
     *
     * @param req HTTP request.
     * @return esp_err_t ESP_OK on success, otherwise an error code.
     */
    static esp_err_t incoming_message_handler(httpd_req_t *req);

    /**
     * @brief Handle the WebSocket handshake.
     *
     * @param req HTTP request.
     * @return esp_err_t ESP_OK on success, otherwise an error code.
     */
    esp_err_t handle_handshake(httpd_req_t *req);

    /**
     * @brief Receive a WebSocket frame.
     *
     * @param req HTTP request.
     * @param ws_pkt WebSocket frame structure.
     * @param buf Buffer to store the frame data.
     * @return esp_err_t ESP_OK on success, otherwise an error code.
     */
    esp_err_t receive_frame(httpd_req_t *req, httpd_ws_frame_t &ws_pkt, uint8_t *&buf);

    /**
     * @brief Process a received WebSocket message.
     *
     * @param req HTTP request.
     * @param ws_pkt WebSocket frame structure.
     * @param buf Buffer containing the message data.
     * @return esp_err_t ESP_OK on success, otherwise an error code.
     */
    esp_err_t process_message(httpd_req_t *req, httpd_ws_frame_t &ws_pkt, uint8_t *buf);

    /**
     * @brief Handle a GET request.
     *
     * @param req HTTP request.
     * @return esp_err_t ESP_OK on success, otherwise an error code.
     */
    esp_err_t handle_get(httpd_req_t *req, std::string &ws_payload_t);

    /**
     * @brief Handle a GET_ALL request.
     *
     * @param req HTTP request.
     * @return esp_err_t ESP_OK on success, otherwise an error code.
     */
    esp_err_t handle_get_all(httpd_req_t *req);

    /**
     * @brief Handle a SET request.
     *
     * @param req HTTP request.
     * @param ws_pkt WebSocket frame structure.
     * @return esp_err_t ESP_OK on success, otherwise an error code.
     */
    esp_err_t handle_set(httpd_req_t *req, std::string &ws_payload_t);

    /**
     * @brief Property change event handler.
     *
     * @param args Pointer to WebSocket instance.
     * @param event_base Event base.
     * @param event_id Event ID.
     * @param event_data Event data.
     */
    static void property_change_event_handler(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data);

    /**
     * @brief Start the HTTP server.
     *
     * @return httpd_handle_t Server handle on success, NULL on failure.
     */
    httpd_handle_t start_webserver();

    /**
     * @brief Stop the HTTP server.
     *
     * @return esp_err_t ESP_OK on success, otherwise an error code.
     */
    esp_err_t stop_webserver();

    /**
     * @brief Register event handlers.
     */
    esp_err_t register_event_handlers();

    /**
     * @brief Unregister event handlers.
     */
    esp_err_t unregister_event_handlers();

    /**
     * @brief WebSocket URI handler structure.
     */
    static const httpd_uri_t ws_uri;

    /**
     * @brief Websocket post to system event loop
     */
    esp_err_t post_system_event(system_event_id_t event, remote_control_event_t message);
};

#endif // WEB_SOCKET_H