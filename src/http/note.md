


message_request
- method enum
- path string_view
- query string -> string_view
- version enum

- header string-> string_view

- body string_view


message_ressponse
- version 
- code 
- detail

- header

- body

message_parser
- now_len
- now_state
- total_len
- is_request

- mehtos_enum
- path_pos
- query_pos
- version_enum
- header_pos
- body_pos

- set_mode
- parser_str
- get_message

message_creater
- set_mode
- set_output

- add_request_start_line
- add_response_start_line
- add_header_pair
- add_body

