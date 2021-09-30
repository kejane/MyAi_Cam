#include <rt_ai_ifperson_model.h>
#include <rt_ai.h>
#include <rt_ai_log.h>
#include <math.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"

#define FM_PIN  GET_PIN(B,12)//FM PIN


#define scaling_x      6.4
#define scaling_y      4.8
static rt_ai_t model = NULL;//为find做准备
extern rt_uint8_t person;
extern int web_get_command(void);
extern int web_post_pic(rt_uint8_t draw_data_src[240][320], int mode);
extern int web_param_test(rt_int8_t array[][320]);


/*
 * 简述：该函数将320X240图像数组用算法缩小并输出50X50的图像数据
 * 参数：320X240的数组输入，50X50的数组输出
 */
void Bilinear_interpolation_algorithm(rt_uint8_t array_in_320X240[][320],rt_uint8_t array_out_50X50[][50])
{
    rt_uint16_t i_in,j_in,i_out,j_out;
    float Px,Py;
    float R2,R1;
    for(j_out=0;j_out<50;j_out++)
    {
        Py=((float)j_out+0.5)*scaling_y-0.5;//计算纵坐标
        j_in=(rt_uint16_t)Py;
        //经计算320*240->50*50不需要判断边界
        for(i_out=0;i_out<50;i_out++)
        {
            Px=((float)i_out+0.5)*scaling_x-0.5;
            i_in=(rt_uint16_t)(((float)i_out+0.5)*scaling_x-0.5);//计算横坐标
            R2=((float)i_in+1.0-Px)*array_in_320X240[j_in][i_in]+(Px-i_in)*array_in_320X240[j_in][i_in+1];//计算R2的灰度值
            R1=((float)i_in+1.0-Px)*array_in_320X240[j_in+1][i_in]+(Px-i_in)*array_in_320X240[j_in+1][i_in+1];
            array_out_50X50[j_out][i_out]=(rt_uint16_t)((Py-j_in)*R1+(j_in+1-Py)*R2);
        }
    }

}
void ai_run_complete(void *arg){
    *(int*)arg = 1;
}


int ifperson_app(rt_uint8_t array_in_320X240[][320])
{
    rt_uint8_t array_out_50X50[50][50]={0};


    /*数据预处理*/
    Bilinear_interpolation_algorithm(array_in_320X240,array_out_50X50);
    /*end*/


	rt_err_t result = RT_EOK;
	uint8_t answer=0;
    int ai_run_complete_flag = 0;//定义两个状态量
    rt_ai_buffer_t *work_buffer = rt_malloc(RT_AI_IFPERSON_WORK_BUFFER_BYTES+RT_AI_IFPERSON_IN_TOTAL_SIZE_BYTES+RT_AI_IFPERSON_OUT_TOTAL_SIZE_BYTES);//
    if(!work_buffer) {rt_kprintf("malloc err\n");return -1;}
    //else rt_kprintf("malloc done\n");
    model = rt_ai_find(RT_AI_IFPERSON_MODEL_NAME);
    if(model == RT_AI_NULL){
        return -1;
    }							//找到模型
    result = rt_ai_init(model, work_buffer);
    //rt_kprintf("ai_init done\n");
    //init the model handle
    if(result != 0){
        return -1;
    }
    //prepare input data
    rt_memcpy(model->input[0], &array_out_50X50[0][0], RT_AI_IFPERSON_IN_1_SIZE_BYTES);
    //rt_kprintf("memcpy done\n");
    result = rt_ai_run(model, ai_run_complete, &ai_run_complete_flag);
    //rt_kprintf("ai_run done\n");
    //process the inference data
    if(ai_run_complete_flag){
        //get inference data
        uint8_t *out = (uint8_t *)rt_ai_output(model, 0);
        answer=out[0];
        //get argmax  
        rt_kprintf("person prediction: %d\n",answer);
        if(answer>180)  person=1;
        else   person=0;
        if(person )  {
            rt_pin_write(FM_PIN, PIN_LOW);
            web_post_pic(array_in_320X240, 0);
        }
        else  rt_pin_write(FM_PIN, PIN_HIGH);
     }   
    rt_free(work_buffer);
    return 0;
}
//MSH_CMD_EXPORT(ifperson_app, person detection demo);

