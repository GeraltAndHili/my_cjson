#include<iostream>
#include<cassert>
#include<cstdio>
#include<cstring>
#include"myleptjson.h"
#include"myleptjson.c"
//测试true
void test_mylept_parse_true(){
    bool all_ok = true;
    lept_value v;
    mylept_init(&v);

    const char *json_true ="true";
    const char *p = json_true;
    int result=mylept_parse_true(&v,&p);
    if(result!=LEPT_PARSE_OK || v.type!=LEPT_TRUE){
        std::printf("test_mylept_parse_true: FAIL (input: \"%s\")\n", json_true);
        all_ok = false;
    }

    const char *json_invalid="trruuee";
    p = json_invalid;
    result=mylept_parse_true(&v,&p);
    if(result!=LEPT_PARSE_INVALID_VALUE){
        std::printf("test_mylept_parse_true: FAIL (input: \"%s\")\n", json_invalid);
        all_ok = false;
    }
    if(all_ok){
        std::printf("test_mylept_parse_true: PASS\n");
    }
}
//测试数字
void test_mylept_parse_number(){
    bool all_ok=true;
    lept_value v;
    mylept_init(&v);

    const char * json_int="12345";
    int result=mylept_parse_number(&v,&json_int);
    if(result!=LEPT_PARSE_OK||v.u.n!=12345){
        printf("整数读取测试失败:(输入:\"%s\")",json_int);
        all_ok=false;
    }

    const char *json_neg_int = "-12345";
    result = mylept_parse_number(&v, &json_neg_int);
    if (result != LEPT_PARSE_OK || v.u.n != -12345) {
        std::printf("test_mylept_parse_number: FAIL (input: \"%s\")\n", json_neg_int);
        all_ok = false;
    }

    // 测试数字0
    const char *json_zero = "0";
    result = mylept_parse_number(&v, &json_zero);
    if (result != LEPT_PARSE_OK || v.u.n != 0) {
        std::printf("test_mylept_parse_number: FAIL (input: \"%s\")\n", json_zero);
        all_ok = false;
    }

    // 测试纯小数
    const char *json_float = "123.456";
    result = mylept_parse_number(&v, &json_float);
    if (result != LEPT_PARSE_OK || v.u.n != 123.456) {
        std::printf("test_mylept_parse_number: FAIL (input: \"%s\")\n", json_float);
        all_ok = false;
    }

    // 测试带小数的负数
    const char *json_neg_float = "-123.456";
    result = mylept_parse_number(&v, &json_neg_float);
    if (result != LEPT_PARSE_OK || v.u.n != -123.456) {
        std::printf("test_mylept_parse_number: FAIL (input: \"%s\")\n", json_neg_float);
        all_ok = false;
    }

    // 测试正数的科学计数法
    const char *json_scientific_pos = "1.23e10";
    result = mylept_parse_number(&v, &json_scientific_pos);
    if (result != LEPT_PARSE_OK || v.u.n != 1.23e10) {
        std::printf("test_mylept_parse_number: FAIL (input: \"%s\")\n", json_scientific_pos);
        all_ok = false;
    }

    // 测试负数的科学计数法
    const char *json_scientific_neg = "-1.23e-10";
    result = mylept_parse_number(&v, &json_scientific_neg);
    if (result != LEPT_PARSE_OK || v.u.n != -1.23e-10) {
        std::printf("test_mylept_parse_number: FAIL (input: \"%s\")\n", json_scientific_neg);
        all_ok = false;
    }
    if(all_ok){
        printf("所有数字类型解析测试:PASS\n");
    }
}

//测试字符串
void test_mylept_parse_string(){
    bool all_ok = true;
    lept_value v;
    mylept_init(&v);

    const char *json_string = "\"吴金朋\"";
    int result = mylept_parse(&v, json_string);
    if(result != LEPT_PARSE_OK || v.type != LEPT_STRING || std::strcmp(v.u.s.s, "吴金朋") != 0){
        std::printf("test_mylept_parse_string: FAIL (input: %s)\n", json_string);
        all_ok = false;
    }
    mylept_free(&v);

    if(all_ok){
        std::printf("test_mylept_parse_string: PASS\n");
    }
}

//测试数组（内容与 test.json 一致）
void test_mylept_parse_array(){
    bool all_ok = true;
    lept_value v;
    mylept_init(&v);

    const char *json_array =
        "[\"吴金朋\",29,\"男\",\"中国\",true,null,{\"major\":\"人工智能\",\"weight_kg\":61,\"height_cm\":167}]";
    int result = mylept_parse(&v, json_array);
    if(result != LEPT_PARSE_OK || v.type != LEPT_ARRAY || v.u.a.len != 7){
        std::printf("test_mylept_parse_array: FAIL (input: %s)\n", json_array);
        all_ok = false;
    }
    mylept_free(&v);

    if(all_ok){
        std::printf("test_mylept_parse_array: PASS\n");
    }
}

//测试对象
void test_mylept_parse_object(){
    bool all_ok = true;
    lept_value v;
    mylept_init(&v);

    const char *json_object = "{\"name\":\"吴金朋\",\"age\":29}";
    int result = mylept_parse(&v, json_object);
    if(result != LEPT_PARSE_OK || v.type != LEPT_OBJECT || v.u.o.len != 2){
        std::printf("test_mylept_parse_object: FAIL (input: %s)\n", json_object);
        all_ok = false;
    }
    mylept_free(&v);

    if(all_ok){
        std::printf("test_mylept_parse_object: PASS\n");
    }
}
int main(){
    //test_mylept_parse_true();
    // test_mylept_parse_number();
    // test_mylept_parse_string();
    // test_mylept_parse_array();
    // test_mylept_parse_object();

    // 从文件读取 JSON 并解析/序列化输出
    {
        const char *path = "../test.json";
        FILE *fp = std::fopen(path, "rb");
        if (!fp) {
            std::printf("open %s failed\n", path);
        } else {
            std::fseek(fp, 0, SEEK_END);
            long size = std::ftell(fp);
            std::fseek(fp, 0, SEEK_SET);
            if (size > 0) {
                char *buf = (char *)std::malloc((size_t)size + 1);
                size_t n = std::fread(buf, 1, (size_t)size, fp);
                buf[n] = '\0';
                lept_value v;
                mylept_init(&v);
                int ret = mylept_parse(&v, buf);
                if (ret != LEPT_PARSE_OK) {
                    std::printf("parse %s failed: %d\n", path, ret);
                } else {
                    size_t out_len = 0;
                    char *out = mylept_stringify(&v, &out_len);
                    if (out) {
                        std::printf("stringify %s: %s\n", path, out);
                        std::free(out);
                    } else {
                        std::printf("stringify %s failed\n", path);
                    }
                }
                mylept_free(&v);
                std::free(buf);
            }
            std::fclose(fp);
        }
    }

    
    return 0;
}
