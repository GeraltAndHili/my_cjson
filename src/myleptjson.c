#include"myleptjson.h"
#include<stdlib.h>//malloc free
#include<string.h>//memcpy
#include <stdio.h>
#include <assert.h>
//辅助结构体，用于字符串化过程中的动态字符串拼接
typedef struct {
    char *s;
    size_t len;
    size_t cap;
} lept_stringify_context;
//辅助函数：确保字符串缓冲区有足够的容
static int lept_context_reserve(lept_stringify_context *c, size_t size){//第二个参数是需要增加的容量
    size_t new_cap;
    char *new_s;
    if(c->len + size <= c->cap)//
        return 1;
    new_cap = c->cap ? c->cap : 256;//这一句的意思是如果cap为0就初始化为256
    while(new_cap < c->len + size)
        new_cap *= 2;
    new_s = (char *)realloc(c->s, new_cap);
    if(new_s == NULL)
        return 0;
    c->s = new_s;
    c->cap = new_cap;
    return 1;
}
//辅助函数：向字符串缓冲区中添加字符串或字符
static int lept_context_puts(lept_stringify_context *c, const char *s, size_t len){//把字符串s的前len个字符放入c中
    if(!lept_context_reserve(c, len))
        return 0;
    memcpy(c->s + c->len, s, len);
    c->len += len;
    return 1;
}

static int lept_context_putc(lept_stringify_context *c, char ch){//把字符ch放入c中
    if(!lept_context_reserve(c, 1))
        return 0;
    c->s[c->len++] = ch;
    return 1;
}




void mylept_init(lept_value * v){
    memset(v,0,sizeof(*v));
    v->type=LEPT_NULL;
}
void mylept_free(lept_value * v){
    switch (v->type){
        case LEPT_STRING:
            free(v->u.s.s);//释放字符串内存
            v->u.s.s=NULL;//防止悬空指针
            mylept_init(v);//重新初始化
            break;
        //等待补充数组等其他类型的释放

        case LEPT_ARRAY:
            for(size_t i=0;i<v->u.a.len;i++){
                mylept_free(&(v->u.a.a[i]));//注意：这里使用递归方式调用free函数来释放数组中每一个元素
            }
            free(v->u.a.a);
            v->u.a.a=NULL;//防止野指针
            mylept_init(v);
            break;

        case LEPT_OBJECT:
            for(size_t i=0;i<v->u.o.len;i++){
                free(v->u.o.m[i].key.s);
                mylept_free(&(v->u.o.m[i].v));
            }
            free(v->u.o.m);
            v->u.o.m=NULL;
            mylept_init(v);
            break;
        default:
            mylept_init(v);
            break;
    }
}
//解析函数
int mylept_parse(lept_value * v,const char * json){
    const char * j=json;
    int ret;

    mylept_parse_whitespace(&j);
    if(*j=='\0'){
        mylept_init(v);
        return LEPT_PARSE_OK;
    }
    ret=mylept_parse_value(v,&j);
    if(ret!=LEPT_PARSE_OK)
        return ret;

    mylept_parse_whitespace(&j);
    if(*j!='\0')
        return LEPT_PARSE_INVALID_VALUE;
    return LEPT_PARSE_OK;
}

int mylept_parse_value(lept_value *v,const char **j){
    mylept_parse_whitespace(j);
    if(**j=='\0'){
        return LEPT_PARSE_INVALID_VALUE;
    }
    while(**j!='\0'){
        if(**j=='n'){
            return mylept_parse_null(v,j);
        }
        if(**j=='t'){
            return mylept_parse_true(v,j);
        }   
        if(**j=='f'){
            return mylept_parse_false(v,j);
        }   
        if((**j>='0'&&**j<='9')||**j=='-'){
            return mylept_parse_number(v,j);
        }
        if(**j=='"'){
            return mylept_parse_string(v,j);
        }
        if(**j=='['){
            return mylept_parse_array(v,j);
        }
        if(**j=='{'){
            return mylept_parse_object(v,j);
        }
//非法字符
        return LEPT_PARSE_INVALID_VALUE; 
    }
    return LEPT_PARSE_OK;
}

void mylept_parse_whitespace(const char ** j){
    while(**j==' '||**j=='\n'||**j=='\r'||**j=='\t'){
        (*j)++;//跳过当前空白字符
    }
}

int mylept_parse_null(lept_value *v,const  char ** j){
    if(strncmp(*j,"null",4)==0){
        *j+=4;
        if(**j=='\0'||**j==' '||**j=='\n'||**j=='\t'||**j==','||**j==']'||**j=='}'){
            v->type=LEPT_NULL;
            return  LEPT_PARSE_OK;
        }
    }
    return LEPT_PARSE_INVALID_VALUE;
}

int mylept_parse_true(lept_value *v,const  char ** j){
    if(strncmp(*j,"true",4)==0){
        *j+=4;
        if(**j=='\0'||**j==' '||**j=='\n'||**j=='\t'||**j==','||**j==']'||**j=='}'){
            v->type=LEPT_TRUE;
            return LEPT_PARSE_OK;
        }
    }
    return LEPT_PARSE_INVALID_VALUE;
}

int mylept_parse_false(lept_value *v,const  char ** j){
        if(strncmp(*j,"false",5)==0){
        *j+=5;
        if(**j=='\0'||**j==' '||**j=='\n'||**j=='\t'||**j==','||**j==']'||**j=='}'){
            v->type=LEPT_FALSE;
            return LEPT_PARSE_OK;
        }
    }
    return LEPT_PARSE_INVALID_VALUE;
}

int mylept_parse_number(lept_value *v,const  char ** j){
    char * end;
    v->u.n=strtod(*j,&end);
    if(*j==end)
        return LEPT_PARSE_INVALID_VALUE;
    v->type=LEPT_NUMBER;
    *j=end;
    return LEPT_PARSE_OK;
}

    //解析十六进制数到码点的辅助函数
static int lept_parse_hex4(const char *p,unsigned *u){
    *u=0;//累计构造码点值
    for(int i=0;i<4;i++){//【误】不是“最多4位”，而是 JSON \uXXXX 固定4位
        char ch=p[i];
        *u<<=4;//左移4位，为当前4比特腾出位置
        if(ch>='0'&&ch<='9')//三种合法十六进制字符
            *u|=(unsigned)(ch-'0');//将当前半字节拼到 u 低位
        else if(ch>='A'&&ch<='F')
            *u|=(unsigned)(ch-'A'+10);
        else if(ch>='a'&&ch<='f')
            *u|=(unsigned)(ch-'a'+10);
        else
            return LEPT_PARSE_INVALID_UNICODE_HEX;//遇到非法字符直接失败
    }
    return LEPT_PARSE_OK;//成功返回 OK
}
    //将unicode编码转换为utf-8编码的辅助函数
static size_t lept_encode_utf8(char * out,unsigned u){//将码点转换为 UTF-8 字节序列写入 out，返回写入字节数
    if(u<=0x7F){//UTF-8 规则：根据码点范围决定 1~4 字节长度
        out[0]=(char)u;//0xxxxxxx
        return 1;
    }
    else if(u<=0x7FF){
        out[0]=(char)(0xC0|((u>>6)));//110xxxxx
        out[1]=(char)(0x80|(u&0x3F));//10xxxxxx
        return 2;
    }
    else if(u<=0xFFFF){
        out[0]=(char)(0xE0|((u>>12)));//1110xxxx
        out[1]=(char)(0x80|((u>>6)&0x3F));//10xxxxxx
        out[2]=(char)(0x80|(u&0x3F));//10xxxxxx
        return 3;
    }
    else{
        assert(u<=0x10FFFF);//Unicode 最大码点
        out[0]=(char)(0xF0|((u>>18)));//11110xxx
        out[1]=(char)(0x80|((u>>12)&0x3F));//10xxxxxx
        out[2]=(char)(0x80|((u>>6)&0x3F));//10xxxxxx
        out[3]=(char)(0x80|(u&0x3F));//10xxxxxx
        return 4;
    }
}

int mylept_parse_string(lept_value *v,const  char ** j){
    if(**j!='"')
        return LEPT_PARSE_MISS_QUOTATION_MARK;
    (*j)++;

    size_t cap=32;//【误】不是最大长度，而是当前缓冲区容量，后续会按需扩容
    size_t len=0;
    char *start=(char *)malloc(cap);//输出缓冲区，后续不足会扩容
    if(start==NULL)
        return LEPT_PARSE_INVALID_VALUE;
    while(**j!='"'&&**j!='\0'){//字符串未结束前持续读取下一个
        unsigned u=0,u2=0;//u 存码点；u2 用于代理对的低位码点
        char ch=**j;
        if ((unsigned char)ch < 0x20) { // 控制字符必须转义，未转义直接判错
            free(start);
            return LEPT_PARSE_INVALID_STRING_CHAR;
        }        
        if(ch=='\\'){//【误】这里只是 C 里表示“反斜杠”字面量，需要处理转义序列
            (*j)++;//【误】JSON 里只有一个反斜杠字符，C 用'\\'表示它；前进一位指向转义后的字符
            switch (**j){
                case '"': ch='"'; break;
                case '\\': ch='\\'; break;
                case '/': ch='/'; break;
                case 'b': ch='\b'; break;
                case 'f': ch='\f'; break;
                case 'n': ch='\n'; break;
                case 'r': ch='\r'; break;
                case 't': ch='\t'; break;
                case 'u':
                    if(lept_parse_hex4(*j+1,&u)!=LEPT_PARSE_OK){//解析 \u 后 4 位十六进制为码点
                        free(start);
                        return LEPT_PARSE_INVALID_UNICODE_HEX;
                    }
                    (*j) += 4;//【误】只是跳过这4位；是否需要代理对由 u 是否落在高代理范围决定
                    if (u >= 0xD800 && u <= 0xDBFF) { // 高代理：必须后跟 \uXXXX 低代理
                        if ((*j)[1] != '\\' || (*j)[2] != 'u') {//必须以 "\u" 开头
                            free(start);
                            return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
                        }
                        if (lept_parse_hex4(*j + 3, &u2) != LEPT_PARSE_OK) {//解析低代理
                            free(start);
                            return LEPT_PARSE_INVALID_UNICODE_HEX;
                        }
                        if (u2 < 0xDC00 || u2 > 0xDFFF) {//低代理范围校验
                            free(start);
                            return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
                        }
                        u = 0x10000 + (((u - 0xD800) << 10) | (u2 - 0xDC00));//合成真实码点
                        (*j) += 6; // 跳过 \uXXXX（低代理）
                    }
                    if (u > 0x10FFFF) {//超过 Unicode 最大码点，非法
                        free(start);
                        return LEPT_PARSE_INVALID_UNICODE_CODEPOINT;
                    }
                    {//将码点编码为 UTF-8 并写入缓冲
                        char utf8[4];//临时存放 UTF-8 字节
                        size_t n = lept_encode_utf8(utf8, u);//n 为 UTF-8 字节数
                        if (len + n + 1 > cap) {//预留 n 字节 + '\0'
                            cap = (cap + n + 1) * 2;
                            start = (char *)realloc(start, cap);
                            if (!start) return LEPT_PARSE_INVALID_VALUE;
                        }
                        for (size_t k = 0; k < n; k++)
                            start[len++] = utf8[k];
                    }
                    (*j)++;
                    continue;
                default:
                    free(start);
                    return LEPT_PARSE_INVALID_STRING_ESCAPE;
        }
    }
    if(len+2>=cap){//确保能写入当前字符和末尾 '\0'
        cap*=2;
        start=(char *)realloc(start,cap);
        if(start==NULL)
            return LEPT_PARSE_INVALID_VALUE;
    }//接下来如果不是转义字符就直接存储当前字符
    start[len++]=ch;
    (*j)++;
    }
    //【误】这里不应跳过空白；字符串解析只消费到结尾引号，空白交给外层处理
    if(**j!='"'){
        free(start);
        return LEPT_PARSE_MISS_QUOTATION_MARK;
    }
    (*j)++;
    start[len]='\0';//补上 C 字符串结束符，再交给 v 管理
    v->u.s.s=start;
    v->u.s.len=len;
    v->type=LEPT_STRING;
    return LEPT_PARSE_OK;

}

int mylept_parse_array(lept_value *v,const char **j){
    if(**j!='[')
        return LEPT_PARSE_INVALID_VALUE;
    (*j)++;
    mylept_parse_whitespace(j);
    v->type=LEPT_ARRAY;
    v->u.a.len=0;
    v->u.a.a=NULL;
    if(**j==']'){
        (*j)++;
        return LEPT_PARSE_OK;
    }
    while(1){
        lept_value e;
        mylept_init(&e);

        int ret=mylept_parse_value(&e,j);
        if(ret!=LEPT_PARSE_OK){
            mylept_free(&e);
            return ret;
        }
        lept_value * new_arr=(lept_value*)realloc(v->u.a.a,sizeof(lept_value)*(v->u.a.len+1));
        if(new_arr==NULL){
            mylept_free(&e);
            return LEPT_PARSE_INVALID_VALUE;
        }
        v->u.a.a=new_arr;
        v->u.a.a[v->u.a.len++]=e;
        
        mylept_parse_whitespace(j);
        if(**j==','){
            (*j)++;
            mylept_parse_whitespace(j);
            continue;
        }
        if(**j==']'){
            (*j)++;
            return LEPT_PARSE_OK;
        }
        return LEPT_PARSE_INVALID_VALUE;

    }
}

int mylept_parse_object(lept_value *v,const char **j){
    if(**j!='{')
        return LEPT_PARSE_INVALID_VALUE;
    (*j)++;
    mylept_parse_whitespace(j);

    v->type=LEPT_OBJECT;
    v->u.o.m=NULL;
    v->u.o.len=0;

    if(**j=='}'){
        (*j)++;
        return LEPT_PARSE_OK;
    }

    while(1){
        lept_member m;
        lept_value key;
        int ret;

        if(**j!='"')
            return LEPT_PARSE_INVALID_VALUE;
        mylept_init(&key);
        //解析字典的键
        ret=mylept_parse_string(&key,j);
        if(ret!=LEPT_PARSE_OK)
            return ret;

        m.key.s=key.u.s.s;
        m.key.len=key.u.s.len;
        key.u.s.s=NULL;

        mylept_parse_whitespace(j);
        if(**j!=':'){
            free(m.key.s);
            return LEPT_PARSE_INVALID_VALUE;
        }
        (*j)++;
        mylept_parse_whitespace(j);
//解析字典的值
        mylept_init(&m.v);
        ret=mylept_parse_value(&m.v,j);
        if(ret!=LEPT_PARSE_OK){
            free(m.key.s);
            mylept_free(&m.v);
            return ret;
        }

        lept_member *new_m=(lept_member*)realloc(v->u.o.m,sizeof(lept_member)*(v->u.o.len+1));
        if(new_m==NULL){
            free(m.key.s);
            mylept_free(&m.v);
            return LEPT_PARSE_INVALID_VALUE;
        }
        v->u.o.m=new_m;
        v->u.o.m[v->u.o.len++]=m;

        mylept_parse_whitespace(j);
        if(**j==','){
            (*j)++;
            mylept_parse_whitespace(j);
            continue;
        }
        if(**j=='}'){
            (*j)++;
            return LEPT_PARSE_OK;
        }
        return LEPT_PARSE_INVALID_VALUE;
    }
}

static int lept_stringify_string(lept_stringify_context *c, const char *s, size_t len){
    static const char hex[] = "0123456789ABCDEF";
    if(!lept_context_putc(c, '"'))
        return 0;
    for(size_t i = 0; i < len; i++){
        unsigned char ch = (unsigned char)s[i];
        switch(ch){
            case '\"': if(!lept_context_puts(c, "\\\"", 2)) return 0; break;
            case '\\': if(!lept_context_puts(c, "\\\\", 2)) return 0; break;
            case '\b': if(!lept_context_puts(c, "\\b", 2)) return 0; break;
            case '\f': if(!lept_context_puts(c, "\\f", 2)) return 0; break;
            case '\n': if(!lept_context_puts(c, "\\n", 2)) return 0; break;
            case '\r': if(!lept_context_puts(c, "\\r", 2)) return 0; break;
            case '\t': if(!lept_context_puts(c, "\\t", 2)) return 0; break;
            default:
                if(ch < 0x20){
                    char buf[6];
                    buf[0] = '\\';
                    buf[1] = 'u';
                    buf[2] = '0';
                    buf[3] = '0';
                    buf[4] = hex[ch >> 4];
                    buf[5] = hex[ch & 0x0F];
                    if(!lept_context_puts(c, buf, 6)) return 0;
                }else{
                    if(!lept_context_putc(c, (char)ch)) return 0;
                }
        }
    }
    return lept_context_putc(c, '"');
}

static int lept_stringify_value(lept_stringify_context *c, const lept_value *v){
    size_t i;
    switch(v->type){
        case LEPT_NULL:
            return lept_context_puts(c, "null", 4);
        case LEPT_FALSE:
            return lept_context_puts(c, "false", 5);
        case LEPT_TRUE:
            return lept_context_puts(c, "true", 4);
        case LEPT_NUMBER: {
            char buf[32];
            int n = snprintf(buf, sizeof(buf), "%.17g", v->u.n);
            if(n < 0)
                return 0;
            return lept_context_puts(c, buf, (size_t)n);
        }
        case LEPT_STRING:
            return lept_stringify_string(c, v->u.s.s, v->u.s.len);
        case LEPT_ARRAY:
            if(!lept_context_putc(c, '[')) return 0;
            for(i = 0; i < v->u.a.len; i++){
                if(i > 0 && !lept_context_putc(c, ',')) return 0;
                if(!lept_stringify_value(c, &v->u.a.a[i])) return 0;
            }
            return lept_context_putc(c, ']');
        case LEPT_OBJECT:
            if(!lept_context_putc(c, '{')) return 0;
            for(i = 0; i < v->u.o.len; i++){
                if(i > 0 && !lept_context_putc(c, ',')) return 0;
                if(!lept_stringify_string(c, v->u.o.m[i].key.s, v->u.o.m[i].key.len)) return 0;
                if(!lept_context_putc(c, ':')) return 0;
                if(!lept_stringify_value(c, &v->u.o.m[i].v)) return 0;
            }
            return lept_context_putc(c, '}');
        default:
            return 0;
    }
}

char * mylept_stringify(const lept_value * v, size_t * len){
    lept_stringify_context c;
    assert(v != NULL);
    c.s = NULL;
    c.len = 0;
    c.cap = 0;
    if(!lept_stringify_value(&c, v)){
        free(c.s);
        return NULL;
    }
    if(!lept_context_putc(&c, '\0')){
        free(c.s);
        return NULL;
    }
    if(len)
        *len = c.len - 1;
    return c.s;
}
//内容访问函数
lept_type mylept_get_type(const lept_value * v){
    assert(v != NULL);
    return v->type;
}

double mylept_get_number(const lept_value * v){
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->u.n;
}

const char * mylept_get_string(const lept_value * v){
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.s;
}

size_t mylept_get_string_length(const lept_value * v){
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.len;
}

size_t mylept_get_array_len(const lept_value * v){
    assert(v != NULL && v->type == LEPT_ARRAY);
    return v->u.a.len;
}

size_t mylept_get_object_len(const lept_value * v){
    assert(v != NULL && v->type == LEPT_OBJECT);
    return v->u.o.len;
}

const lept_value * mylept_get_array_element(const lept_value * v, size_t index){
    assert(v != NULL && v->type == LEPT_ARRAY);
    assert(index < v->u.a.len);
    return &v->u.a.a[index];
}

const char * mylept_get_object_key(const lept_value * v, size_t index){
    assert(v != NULL && v->type == LEPT_OBJECT);
    assert(index < v->u.o.len);
    return v->u.o.m[index].key.s;
}

size_t mylept_get_object_key_length(const lept_value * v, size_t index){
    assert(v != NULL && v->type == LEPT_OBJECT);
    assert(index < v->u.o.len);
    return v->u.o.m[index].key.len;
}

const lept_value * mylept_get_object_value(const lept_value * v, size_t index){
    assert(v != NULL && v->type == LEPT_OBJECT);
    assert(index < v->u.o.len);
    return &v->u.o.m[index].v;
}
//内容修改函数
void mylept_set_null(lept_value * v){
    assert(v != NULL);
    mylept_free(v);
}

void mylept_set_number(lept_value * v,double n){
    assert(v != NULL);
    mylept_free(v);
    v->u.n=n;
    v->type=LEPT_NUMBER;
}

void mylept_set_string(lept_value * v,const char * s,size_t len){
    assert(v != NULL && (s != NULL || len == 0));
    char * new_s=(char *)malloc(len+1);
    if(len>0)
        memcpy(new_s,s,len);
    new_s[len]='\0';
    mylept_free(v);
    v->u.s.s=new_s;
    v->u.s.len=len;
    v->type=LEPT_STRING;
}









#ifdef MYLEPTJSON_TEST



int main(void) {
    lept_value v;
    mylept_init(&v);
    mylept_free(&v);
    printf("build ok\n");
    return 0;
}

#endif
