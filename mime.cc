typedef enum
{
    e_application,
    e_audio,
    e_font,
    e_example,
    e_image,
    e_message,
    e_model,
    e_multipart,
    e_text,
    e_video,
    e_media_type_max,
}media_type_t;
typedef enum
{
}media_type_application_t;
typedef enum
{
    e_text_plain,
    e_text_css,
    e_text_html,
    e_text_javascript,
    e_image_gif,
    e_image_jpeg,
    e_image_png,
    e_image_svg_xml,
    e_image_x_icon,
}http_mime_t;
char *http_mime_types[] =
{
    "text/plain",
    "text/css",
    "text/html",
    "text/javascript",
    "image/gif",
    "image/jpeg",
    "image/png",
    "image/svg+xml",
    "image/x-icon",
    "audio/midi",
    "audio/mpegi",
    "audio/webm",
    "audio/ogg",
    "audio/wav",
    "video/webm",
    "video/ogg",
    "application/octet-stream",
    "application/pkcs12",
    "application/vnd.mspowerpoint",
    "application/xhtml+xml",
    "application/xml",
    "application/pdf",
};
typedef struct
{
    char *type_name;
    char *subtype_name;
    char *file_ext;
}media_type_t;
media_type_t media_types = 
{
    // type ID,               type name,          subtype name,   file extension
    {  e_application_xml,     "application",      "xml",          "xml"         },
    {  e_application_pdf,     "application",      "pdf",          "pdf"         },
    {  e_text_plain,          "text",             "plain",        "txt"         },
    {  e_text_css,            "text",             "css",          "css"         },
    {  e_text_html,           "text",             "html",         "html"        },
    {  e_text_javascript,     "text",             "javascript",   "js"          },
    {  e_image_gif,           "image",            "gif",          "gif"         },
    {  e_image_jpeg,          "image",            "jpeg",         "jpeg|jpg"    },
    {  e_image_svg_xml,       "image",            "svg+xml",      "svg"         },
    {  e_image_x_icon,        "image",            "x-icon",       "icon"        },
};
