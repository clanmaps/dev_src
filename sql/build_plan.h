/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file build_plan.h
 * @brief main entrance of build logical plan
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_24
 */



#ifndef BUILD_PLAN_H
#define BUILD_PLAN_H

#include "parse_node.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int resolve(ResultPlan* result_plan, ParseNode* node);
extern void destroy_plan(ResultPlan* result_plan);

#ifdef __cplusplus
}
#endif

#endif //BUILD_PLAN_H
