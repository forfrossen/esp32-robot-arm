#include "ResponseHandler.hpp"
static const char *TAG = "MotorResponseHandler";
// ESP_EVENT_DEFINE_BASE(MOTOR_EVENTS);

ResponseHandler::ResponseHandler(
    uint32_t canId,
    std::shared_ptr<MotorContext> context,
    const std::shared_ptr<EventLoops> &event_loops,
    std::shared_ptr<CommandLifecycleRegistry> registry)
    : canId(canId),
      context(context),
      system_event_loop(event_loops->system_event_loop),
      motor_event_loop(event_loops->motor_event_loop),
      command_lifecycle_registry(registry)
{
    ESP_LOGD(TAG, "ResponseHandler constructor called");

    assert(motor_event_loop != nullptr);

    esp_err_t err = esp_event_handler_instance_register_with(
        motor_event_loop,
        MOTOR_EVENTS,
        INCOMING_MESSAGE_EVENT,
        &incoming_message_event_handler,
        this,
        &incoming_message_event_handler_instance);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register event handler: %s", esp_err_to_name(err));
    }

    entry_point = std::make_shared<ResponseHandlerEntry>();
    log_handler = std::make_shared<LogMessageHandler>();
    error_handler = std::make_shared<ErrorCheckHandler>(context, command_lifecycle_registry);
    data_handler = std::make_shared<ResponseDataHandler>(context);
    state_handler = std::make_shared<ReadyStateTransitionHandler>(context);
    lifecycle_handler = std::make_shared<CommandLifecycleHandler>();
    // std::shared_ptr<SetCommandResponseHandler> set_command_response_handler = std::make_shared<SetCommandResponseHandler>(context);

    // Chain the handlers together
    entry_point->set_next(log_handler);
    log_handler->set_next(error_handler);
    error_handler->set_next(data_handler);
    data_handler->set_next(state_handler);
    state_handler->set_next(lifecycle_handler);
};

ResponseHandler::~ResponseHandler() {}

void ResponseHandler::incoming_message_event_handler(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ResponseHandler *instance = static_cast<ResponseHandler *>(args);
    ESP_LOGD(TAG, "GOT MOTOR EVENT FOR MOTOR %lu, EVENT_BASE: %s, EVENT_ID: %lu", instance->canId, event_base, event_id);

    RETURN_IF_NOT(event_base == MOTOR_EVENTS);
    RETURN_IF_NOT(event_id == INCOMING_MESSAGE_EVENT);

    twai_message_t *msg = (twai_message_t *)event_data;

    instance->entry_point->handle_response(*msg);
}

// esp_err_t ResponseHandler::process_message(twai_message_t *msg)
// {
//     check_for_error_and_do_transition(msg);

//     CommandIds commandId = static_cast<CommandIds>(msg->data[0]);

//     switch (commandId)
//     {
//     case SET_WORKING_CURRENT:
//     case SET_HOLDING_CURRENT:
//     case SET_HOME:
//         CHECK_THAT(handle_set_command_response(msg) == ESP_OK);

//     // Handle read commands that return uint16_t
//     case READ_MOTOR_SPEED:
//         CHECK_THAT(handle_read_uint16_response(msg) == ESP_OK);
//         break;
//     case READ_ENCODER_VALUE_CARRY:
//         CHECK_THAT(handle_query_motor_position_response(msg) == ESP_OK);
//         break;
//     case SET_WORK_MODE:
//         CHECK_THAT(handle_set_work_mode_response(msg) == ESP_OK);
//         break;
//     // case SET_WORKING_CURRENT:
//     //     handle_set_current_response(msg);
//     //     break;
//     // case SET_HOME:
//     //     handle_set_home_response(msg);
//     //     break;
//     case QUERY_MOTOR_STATUS:
//         CHECK_THAT(handle_query_status_response(msg) == ESP_OK);
//         break;
//     case RUN_MOTOR_RELATIVE_MOTION_BY_AXIS:
//     case RUN_MOTOR_ABSOLUTE_MOTION_BY_AXIS:
//         CHECK_THAT(handle_set_position_response(msg) == ESP_OK);
//         break;
//     default:
//         ESP_LOGW(TAG, "No response handler defined for code: %02X", msg->data[0]);
//         break;
//     }
//     return ESP_OK;
// }

// bool ResponseHandler::is_response_error(twai_message_t *msg)
// {
//     RETURN_BOOL(msg->data[1] == 0);
// }

// esp_err_t ResponseHandler::handle_received_no_error()
// {
//     if (context->is_recovering() || context->is_init())
//     {
//         CHECK_THAT(context->transition_ready_state(MotorContext::ReadyState::MOTOR_READY) == ESP_OK);
//     }
//     return ESP_OK;
// }

// esp_err_t ResponseHandler::handle_message_error()
// {
//     CHECK_THAT(context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR) == ESP_OK);
//     return ESP_OK;
// }

// esp_err_t ResponseHandler::check_for_error_and_do_transition(twai_message_t *msg)
// {
//     if (is_response_error(msg))
//     {
//         CHECK_THAT(handle_message_error());
//         ESP_LOGD(TAG, "Error response received.");
//     }
//     else
//     {
//         CHECK_THAT(handle_received_no_error());
//         ESP_LOGD(TAG, "Response does not contain an error.");
//     }
//     return ESP_OK;
// }

// esp_err_t ResponseHandler::print_unknown_response_code(twai_message_t *msg)
// {
//     auto status = msg->data[1];
//     ESP_LOGW(TAG, "Unknown status response: %02X", status);
//     for (int i = 0; i < msg->data_length_code; i++)
//     {
//         ESP_LOGD(TAG, "Data[%d] - raw: %d \t - hex: %02X", i, msg->data[i], msg->data[i]);
//     }
//     return ESP_OK;
// }

// esp_err_t ResponseHandler::handle_set_command_response(twai_message_t *msg)
// {
//     uint8_t status = msg->data[1];

//     if (status == 0)
//     {
//         ESP_LOGE(TAG, "Set command failed for command: %s", GET_MSGCMD(&msg));
//     }

//     else
//     {
//         ESP_LOGD(TAG, "Set command successful for command: %s", GET_MSGCMD(&msg));
//     }
//     return ESP_OK;
// }

// esp_err_t ResponseHandler::handle_read_uint16_response(twai_message_t *msg)
// {
//     uint16_t response = (msg->data[1] << 8) | msg->data[2];
//     ESP_LOGD(TAG, "Response: %u", response);
//     return ESP_OK;
// }

// esp_err_t ResponseHandler::handle_query_status_response(twai_message_t *msg)
// {
//     uint8_t status = msg->data[1];

//     switch (status)
//     {
//     case 0:
//         ESP_LOGE(TAG, "Abfrage fehlgeschlagen");
//         context->set_motor_moving_state(MotorContext::MovingState::UNKNOWN);
//         break;
//     case 1:
//         ESP_LOGD(TAG, "Motor gestoppt");
//         context->set_motor_moving_state(MotorContext::MovingState::STOPPED);
//         break;
//     case 2:
//         ESP_LOGD(TAG, "Motor beschleunigt");
//         context->set_motor_moving_state(MotorContext::MovingState::ACCELERATING);
//         break;
//     case 3:
//         ESP_LOGD(TAG, "Motor verlangsamt");
//         context->set_motor_moving_state(MotorContext::MovingState::DECELERATING);
//         break;
//     case 4:
//         ESP_LOGD(TAG, "Motor volle Geschwindigkeit");
//         context->set_motor_moving_state(MotorContext::MovingState::FULL_SPEED);
//         break;
//     case 5:
//         ESP_LOGD(TAG, "Motor fährt nach Hause");
//         context->set_motor_moving_state(MotorContext::MovingState::HOMING);
//         break;
//     case 6:
//         ESP_LOGD(TAG, "Motor wird kalibriert");
//         context->set_motor_moving_state(MotorContext::MovingState::CALIBRATING);
//         break;
//     default:
//         ESP_LOGW(TAG, "Unknown status response: %02X", status);
//         context->set_motor_moving_state(MotorContext::MovingState::UNKNOWN);
//         break;
//     }
//     return ESP_OK;
// }

// esp_err_t ResponseHandler::handle_query_motor_position_response(twai_message_t *msg)
// {
//     uint8_t *data = msg->data;

//     uint32_t carry = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
//     uint16_t encoderValue = (data[5] << 8) | data[6];

//     // Optional: Überprüfen, ob Value außerhalb des gültigen Bereichs liegt
//     if (encoderValue > 0x3FFF)
//     {
//         carry += 1;
//         encoderValue -= 0x4000;
//     }
//     else if (encoderValue < 0)
//     {
//         carry -= 1;
//         encoderValue += 0x4000;
//     }

//     // // Werte im Kontext speichern
//     // context->set_carry_value(carry);
//     // context->set_encoder_value(encoderValue);

//     // // Absolute Position berechnen
//     int64_t absolutePosition = ((int64_t)carry << 14) + encoderValue;
//     // context->set_absolute_position(absolutePosition);

//     ESP_LOGD(TAG, "Carry value: %ld", carry);
//     ESP_LOGD(TAG, "Encoder value: %u", encoderValue);
//     ESP_LOGD(TAG, "Absolute position: %lld", absolutePosition);
//     return ESP_OK;
// }

// esp_err_t ResponseHandler::handle_query_motor_speed_response(twai_message_t *msg)
// {
//     uint16_t speed = (msg->data[2] << 16) | (msg->data[3] << 8) | msg->data[4];
//     ESP_LOGD(TAG, "Speed: %u", speed);
//     return ESP_OK;
// }

// esp_err_t ResponseHandler::handle_set_position_response(twai_message_t *msg)
// {
//     uint8_t status = msg->data[1];

//     std::string F5Status;
//     switch (status)
//     {
//     case 0:
//         F5Status = "Run failed";
//         ESP_LOGD(TAG, "Run failed");
//         // context->set_motor_moving_state(MotorContext::MovingState::ERROR);
//         break;
//     case 1:
//         F5Status = "Run starting";
//         ESP_LOGD(TAG, "Run starting");
//         // fsm_moving.set_state(MotorContext::ReadyState::);
//         break;
//     case 2:
//         F5Status = "Run complete";
//         ESP_LOGD(TAG, "Run complete");
//         // fsm_moving.set_state(MotorControllerFSM::State::COMPLETED);
//         break;
//     case 3:
//         F5Status = "End limit stopped";
//         ESP_LOGD(TAG, "End limit stopped");
//         // fsm_moving.set_state(MotorControllerFSM::State::COMPLETED);
//         break;
//     default:
//         F5Status = "Unknown status";
//         ESP_LOGW(TAG, "Unknown status");
//         // fsm_moving.set_state(MotorControllerFSM::State::ERROR);
//         break;
//     }
//     return ESP_OK;
// }

// esp_err_t ResponseHandler::handle_set_home_response(twai_message_t *msg)
// {
//     uint8_t status = msg->data[1];
//     std::string statusMessage;

//     switch (status)
//     {
//     case 0:
//         statusMessage = "Set home failed.";
//         break;
//     case 1:
//         statusMessage = "Set home in progress...";
//         break;
//     case 2:
//         statusMessage = "Set home completed.";
//         break;
//     default:
//         statusMessage = "Unknown status.";
//         break;
//     }

//     ESP_LOGD(TAG, "Set Home Response: %s", statusMessage.c_str());
//     return ESP_OK;
// }

// esp_err_t ResponseHandler::handle_set_work_mode_response(twai_message_t *msg)
// {
//     if (msg->data[1] == 1)
//     {
//         ESP_LOGD(TAG, "Set Work Mode: Success");
//     }
//     else
//     {
//         ESP_LOGE(TAG, "Set Work Mode: Failed");
//     }
//     return ESP_OK;
// }

// esp_err_t ResponseHandler::handle_set_current_response(twai_message_t *msg)
// {
//     if (msg->data[1] == 1)
//     {
//         ESP_LOGD(TAG, "Set Current: Success");
//     }
//     else
//     {
//         ESP_LOGE(TAG, "Set Current: Failed");
//     }
//     return ESP_OK;
// }
