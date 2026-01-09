# my_cjson

一个手写的 C JSON 解析/序列化练习库。

## 基础环境需求
- CMake >= 3.12
- C 编译器（gcc/clang/msvc 均可）

## 目录结构
- `src/myleptjson.c`：核心实现
- `src/myleptjson.h`：对外接口
- `src/test.cpp`：简单测试与文件解析示例
- `test.json`：测试数据

## 构建与运行（CMake）
```sh
cmake -S . -B build
cmake --build build
./build/myleptjson_test
```

## 最小使用示例
```c
#include "myleptjson.h"

int main(void) {
    lept_value v;
    mylept_init(&v);
    if (mylept_parse(&v, "[1, true, \"hi\"]") == LEPT_PARSE_OK) {
        size_t out_len = 0;
        char *out = mylept_stringify(&v, &out_len);
        /* use out... */
        free(out);
    }
    mylept_free(&v);
    return 0;
}
```

## 编写过程中需要的基础知识
1. 枚举值的小知识：
   为什么可以使用枚举值来定义变量？
   因为 C 语言实际上把 enum 当作整数类型。
   例如 `lept_value a = lept_null;` 实际上就是把 `a` 赋值为 0，
   因为 `lept_null` 是枚举的第一个值。
