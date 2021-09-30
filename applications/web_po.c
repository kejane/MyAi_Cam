/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-09-26     JingMing       the first version
 */
#include "web_po.h"
#include <webclient.h>

struct webclient_session *session_get = RT_NULL, *session_post = RT_NULL;
unsigned char *buffer = RT_NULL;
int web_client_init(void){

    session_post = webclient_session_create(POST_HEADER_BUFSZ);
    session_get = webclient_session_create(POST_HEADER_BUFSZ);

    buffer = (unsigned char *) web_malloc(GET_RESP_BUFSZ);

    return 0;
}
INIT_APP_EXPORT(web_client_init);

//完成体
static void process_pic_data(const rt_uint8_t original_data[][320], char *process_data){
    int count=76800;
#ifdef _USE_BEMFA_
    for(int cnt=0;cnt<1078;cnt++)
        rt_snprintf(process_data+cnt,2,"%c",(char)bmp_header_[cnt]);
    for(int cnt=1078;cnt<(76800+1078);cnt++){
        rt_snprintf(process_data+cnt,2,"%c",(char)(*(*(original_data)+count)));
        count++;
    }
#elif defined _USE_ONENET_
    for(int cnt=0;cnt<1078;cnt++)
        process_data[cnt]=bmp_header_[cnt];
    for(int cnt=1078;cnt<77878;cnt++){
        process_data[cnt]=*(original_data[0]+count);
        count--;
    }
#ifdef _LOG_E_OUT_
    rt_kprintf("enter process_pic_data\r\n");
#endif

#endif
}
//完成体
static int web_post_action(const char *uri, const char *post_data){
    unsigned char *response = RT_NULL;
    int index, ret = 0;
    int bytes_read, resp_status;

    webclient_header_fields_add(session_post, "Content-Length: %d\r\n", (77878));//strlen(post_data)
    #ifdef _USE_BEMFA_
        webclient_header_fields_add(session_post, _content_type);
        webclient_header_fields_add(session_post, _authorization);
        webclient_header_fields_add(session_post, _topic);
    #elif defined _USE_ONENET_
        webclient_header_fields_add(session_post, _api_key);
        webclient_header_fields_add(session_post, _content_type);
    #endif

    response = (unsigned char *) web_malloc(POST_RESP_BUFSZ);
    if(response == RT_NULL){
        rt_kprintf("webClient: memory for response apply failed!\r\n");
        ret = -RT_ENOMEM;
        goto __exit;
    }

#ifdef _LOG_E_OUT_
    rt_kprintf("enter webclient_post\r\n");
#endif
    if ((resp_status = webclient_post(session_post, uri, post_data, (77878))) != 200)
    {
        rt_kprintf("webClient: webclient POST request failed, response(%d) error.\n", resp_status);
        ret = -RT_ERROR;
        goto __exit;
    }

    rt_kprintf("webClient: webclient post response data: \n");
    do
    {
#ifdef _LOG_E_OUT_
    rt_kprintf("enter webclient_read\r\n");
#endif
        bytes_read = webclient_read(session_post, response, POST_RESP_BUFSZ);
        if (bytes_read <= 0)
        {
            break;
        }

        for (index = 0; index < bytes_read; index++)
        {
            rt_kprintf("%c", response[index]);
        }
    } while (1);

    rt_memset(session_post->header->buffer,0x00,session_post->header->size);
    session_post->header->length = 0;

__exit:

        web_free(response);

    return ret;
}
//102行开始到104行
int web_post_pic(rt_uint8_t draw_data_src[][320], int mode){
#ifdef _LOG_E_OUT_
    rt_kprintf("entered web_post_pic\r\n");
#endif
    char *uri = RT_NULL, *process_data = RT_NULL;
    int ret = 0;

    if(mode == 0)
        uri = web_strdup(POST_LOCAL_URI);
    else if(mode == 1)
        uri = web_strdup(POST_ANOTHER_URI);

    if(uri == RT_NULL){
        rt_kprintf("webClient: no memory for create post request uri buffer.\n");
        return -RT_ENOMEM;
    }

#ifdef _LOG_E_OUT_
    rt_kprintf("got to malloc\r\n");
#endif
    process_data = (char *)rt_malloc(78000);
    rt_memset((void *)process_data,0,78000);//_mem_to_malloc_
#ifdef _LOG_E_OUT_
    rt_kprintf("malloc finished\r\n");
#endif

#ifdef _LOG_E_OUT_
    rt_kprintf("enter process_pic_dataa\r\n");
#endif
    process_pic_data(draw_data_src, process_data);


#ifdef _OUTPUT_LOG_
    for(int cnt=0;cnt<(77878);cnt++){
        rt_kprintf("%x",process_data[cnt]);
    }
#endif

#ifdef _LOG_E_OUT_
    rt_kprintf("enter web_post_action\r\n");
#endif
    ret = web_post_action(uri,process_data);

    web_free(uri);
    rt_free(process_data);

    return ret;
}

/*******from here*******/
static int web_get_action(const char *uri){
    int index, button_value=0;
    int bytes_read, resp_status;
    int content_length = -1;

    webclient_header_fields_add(session_get, _api_key);

    /* send GET request by default header */
    if ((resp_status = webclient_get(session_get, uri)) != 200)
    {
        rt_kprintf("webclient GET request failed, response(%d) error.\n", resp_status);
        goto __exit;
    }

    rt_kprintf("webclient get response data: \n");

    content_length = webclient_content_length_get(session_get);
    if (content_length < 0)
    {
        rt_kprintf("webclient GET request type is chunked.\n");
        do
        {
            bytes_read = webclient_read(session_get, buffer, GET_RESP_BUFSZ);
            if (bytes_read <= 0)
            {
                break;
            }

            for (index = 0; index < bytes_read; index++)
            {
                rt_kprintf("%c", buffer[index]);
            }
        } while (1);

        rt_kprintf("\n");
    }
    else
    {
        int content_pos = 0;

        do
        {
            bytes_read = webclient_read(session_get, buffer,
                    content_length - content_pos > GET_RESP_BUFSZ ?
                            GET_RESP_BUFSZ : content_length - content_pos);
            if (bytes_read <= 0)
            {
                break;
            }
#ifdef _OUTPUT_RESPONSE_
            for (index = 0; index < bytes_read; index++)
            {
                rt_kprintf("%c", buffer[index]);
            }
#endif
            content_pos += bytes_read;
        } while (content_pos < content_length);

        rt_kprintf("\n");
    }

    /*************/
    button_value = buffer[110] - 48;
    rt_memset(session_get->header->buffer,0x00,session_get->header->size);
    session_get->header->length = 0;
    /*************/

    __exit:

    return button_value;
}

//直接使用,返回值为1或0
int web_get_command(void){
    char *uri = RT_NULL;
    int ret = 0;

    uri = web_strdup(GET_LOCAL_URI);
    if(uri == RT_NULL)
    {
        rt_kprintf("no memory for create get request uri buffer.\n");
        return -RT_ENOMEM;
    }

    ret = web_get_action(uri);

    return ret;
}
/*********get here**********/
