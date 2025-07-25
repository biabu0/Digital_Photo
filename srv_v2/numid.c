#include <alsa/asoundlib.h>
#include <stdio.h>

int main() {
    snd_ctl_t *handle;
    snd_ctl_elem_list_t *list;
    int err;

    // 打开声卡控制接口
    if ((err = snd_ctl_open(&handle, "hw:0", 0)) < 0) {
        fprintf(stderr, "无法打开声卡: %s\n", snd_strerror(err));
        return err;
    }

    // 分配控件列表内存
    snd_ctl_elem_list_alloca(&list);

    // 获取控件列表
    if ((err = snd_ctl_elem_list(handle, list)) < 0) {
        fprintf(stderr, "无法获取控件列表: %s\n", snd_strerror(err));
        snd_ctl_close(handle);
        return err;
    }

    // 获取控件数量并检查有效性
    int count = snd_ctl_elem_list_get_count(list);
    if (count <= 0) {
        fprintf(stderr, "未找到任何声卡控件\n");
        snd_ctl_close(handle);
        return -1;
    }

    printf("找到 %d 个控件\n", count);

    // 遍历控件（确保索引范围正确）
    for (int i = 0; i < count; i++) {
        snd_ctl_elem_id_t *id;
        snd_ctl_elem_id_alloca(&id);
        
        // 直接调用函数，无需检查返回值（因函数返回 void）
        snd_ctl_elem_list_get_id(list, i, id);
        
        printf("控件 %d: numid=%d, name=%s\n",
               i,
               snd_ctl_elem_id_get_numid(id),
               snd_ctl_elem_id_get_name(id));
    }

    snd_ctl_close(handle);
    return 0;
}