//
//  main.m
//  debug
//
//  Created by October on 07/02/2018.
//

#include <CoreFoundation/CFRunLoop.h>
#include <objc/objc-class.h>
#include <Block.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "base.h"

static inline void run_objc_inspect(void);

static inline void runLoop_source_perform(void *info) {
    let perform = (void (^)(void))info;
    !perform ?: (perform)();
    run_objc_inspect();
}

static let runLoop_get_source = ^(id perform_block) {
    static var src0 = (CFRunLoopSourceRef)nil;
    var src_ctx = (CFRunLoopSourceContext) {
        0, perform_block, nil, nil, nil, nil, nil, nil, nil, runLoop_source_perform
    };
    if (src0 != nil) {
        CFRunLoopSourceGetContext(src0, &src_ctx);
        src_ctx.info = perform_block;
        goto done;
    }
    src0 = CFRunLoopSourceCreate(kCFAllocatorDefault, 0, &src_ctx);
done:
    return src0;
};

let objc_inspect = ^{
    static var cls_ls = (Class *)nil;
    if (!cls_ls) {
        printf("Class list: \n");
        var cls_c = (uint)0;
        cls_ls = objc_copyClassList(&cls_c);
        for (var i = (uint)0; i < cls_c; i++) {
            let cls = cls_ls[i];
            printf("%d. %s\n", i, class_getName(cls));
        }
    }
    
    printf("Please specify which Class to be inspect: ");
    let buf_size = (size_t)100;
    var c_str = (char *)malloc(sizeof(char) * buf_size);
    defer {
        free(c_str);
    };
    fgets(c_str, buf_size, stdin);
    var cls = objc_getClass(strtok(c_str, "\n"));
    
    printf("Class Method list: \n");
    var cls_mtd_c = (uint)0;
    var cls_mtd_ls = class_copyMethodList(object_getClass((id)cls), &cls_mtd_c);
    defer {
        free(cls_mtd_ls);
    };
    for (var i = (uint)0; i < cls_mtd_c; i++) {
        let method = cls_mtd_ls[i];
        printf("%d. +%s\n", i, sel_getName(method_getName(method)));
    }
    
    printf("Instance Method list: \n");
    var ins_mtd_c = (uint)0;
    var ins_mtd_ls = class_copyMethodList(cls, &ins_mtd_c);
    defer {
        free(ins_mtd_ls);
    };
    for (var i = (uint)0; i < ins_mtd_c; i++) {
        let method = ins_mtd_ls[i];
        printf("%d. -%s\n", i, sel_getName(method_getName(method)));
    }
    
    var sel_str = (char *)malloc(sizeof(char) *buf_size);
    defer {
        free(sel_str);
    };
    var obj = (id)cls;
    goto input;
    do {
        let sel = sel_getUid(sel_str);
        let is_cls = object_isClass(obj);
        obj = objc_msgSend(obj, sel) ?: obj;
        printf("Message: %c[%s %s] \n\n", is_cls ? '+' : '-', class_getName(object_getClass(obj)), sel_getName(sel));
    input:
        printf("Please specify sel, type enter for terminate: ");
        fgets(sel_str, buf_size, stdin);
    } while (strtok(sel_str, "\n"));
    
    printf("Awaiting for next inspection \n\n");
};

static inline void run_objc_inspect(void) {
    let copyed_blk = _Block_copy((id)objc_inspect);
    var src0 = runLoop_get_source(copyed_blk);
    if (CFRunLoopContainsSource(CFRunLoopGetCurrent(), src0, kCFRunLoopDefaultMode)){
        goto done;
    }
    CFRunLoopAddSource(CFRunLoopGetCurrent(), src0, kCFRunLoopDefaultMode);
done:
    CFRunLoopSourceSignal(src0);
    CFRunLoopWakeUp(CFRunLoopGetCurrent());
}

int main(int argc, const char * argv[]) {
    run_objc_inspect();
    CFRunLoopRun();
    return 0;
}

