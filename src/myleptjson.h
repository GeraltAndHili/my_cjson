#ifndef MYLEPTJSON_h__
#define MYLEPTJSON_h__
#include<stddef.h>//size_t需要包含这个头文件
#define LEPT_PARSE_OK 0
#define LEPT_PARSE_INVALID_VALUE 1//无效
#define LEPT_PARSE_MISS_QUOTATION_MARK 2//缺少引号
#define LEPT_PARSE_INVALID_STRING_ESCAPE 3//无效的字符串转义字符
#define LEPT_PARSE_INVALID_UNICODE_HEX 4//无效的unicode十六进制
#define LEPT_PARSE_INVALID_UNICODE_SURROGATE 5//无效的unicode代理对
#define LEPT_PARSE_INVALID_STRING_CHAR 6//无效的字符串字符
#define LEPT_PARSE_INVALID_UNICODE_CODEPOINT 7//无效的unicode码点


typedef enum{
    LEPT_NULL,
    LEPT_FALSE,
    LEPT_TRUE,
    LEPT_NUMBER,
    LEPT_STRING,
    LEPT_ARRAY,
    LEPT_OBJECT//可以理解为字典
}lept_type;//规定了所有类型，共六种：数值、bool(通常分true和false)、字符串、数组、字典、空值null

typedef struct lept_value lept_value;
typedef struct lept_member lept_member;


struct lept_value{
    lept_type type;
    union{
        double n;
        struct {
            char *s;
            size_t len;
        }s;
        struct {
            lept_value *a;
            size_t len;
        }a;
        struct {
            lept_member * m;
            size_t len;
        } o;
    }u;//为什么不需要专门为null、true、false设置成员，因为它们不需要存储额外的信息，他们的信息实际已经存储在lept_type type中了
};

struct lept_member{
    struct k{
            char *s;//这里不加const是因为对象的key在解析的时候需要临时修改字符串
            size_t len;
    }key;
    lept_value v;//这里一般不用指针
};//这里是为了lept_value中的对象(字典)类型服务的，key是键，v是值
//好好思考一下lept_member和lept_value的关系，并举例来进行嵌套调用的理解

//开始编写接口函数，这些接口函数是给用户调用的
    //一些基本操作辅助函数
void mylept_init(lept_value * v);
void mylept_free(lept_value * v);
void mylept_parse_whitespace(const char ** j);
    //解析函数
int mylept_parse(lept_value * v,const char * json);
    //外层接口函数
int mylept_parse_value(lept_value *v,const char ** j);
int mylept_parse_null(lept_value *v,const  char ** j);
int mylept_parse_true(lept_value *v,const  char ** j);
int mylept_parse_false(lept_value *v,const  char ** j);
int mylept_parse_number(lept_value *v,const  char ** j);
int mylept_parse_string(lept_value *v,const  char ** j);
int mylept_parse_array(lept_value *v,const  char ** j);
int mylept_parse_object(lept_value *v,const  char ** j);
char * mylept_stringify(const lept_value * v, size_t * len);
    //内容访问函数
lept_type mylept_get_type(const lept_value * v);
double mylept_get_number(const lept_value * v);
const char * mylept_get_string(const lept_value * v);
size_t mylept_get_string_length(const lept_value * v);
size_t mylept_get_array_len(const lept_value * v);
size_t mylept_get_object_len(const lept_value * v);
const lept_value * mylept_get_array_element(const lept_value * v, size_t index);
const char * mylept_get_object_key(const lept_value * v, size_t index);
size_t mylept_get_object_key_length(const lept_value * v, size_t index);
const lept_value * mylept_get_object_value(const lept_value * v, size_t index);
    //内容修改函数
void mylept_set_null(lept_value * v);
void mylept_set_number(lept_value * v,double n);
void mylept_set_string(lept_value * v,const char * s,size_t len);



#endif
