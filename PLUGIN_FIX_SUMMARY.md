# 插件系统修复总结

## 问题描述

从用户提供的截图发现：
1. **插件显示"未命名插件"** - 插件的元数据（名称、描述、版本等）没有被正确提取
2. **按钮显示"Button"** - 插件的UI模板没有正确渲染，所有按钮都显示默认文本"Button"

## 根本原因分析

后端的元数据提取函数 `extract_plugin_meta()` 存在以下问题：

1. **没有定位到正确的解析起点** - 直接在整个文件中搜索 `name:`，可能匹配到注释或其他位置
2. **不支持换行符** - 只跳过空格和制表符，不跳过换行符 `\n` 和 `\r`
3. **不支持转义字符** - 无法处理字符串中的转义字符如 `\n`、`\t` 等

## 已完成的修复

### 1. 改进后端元数据提取逻辑

**文件**: `src/system/plugin.c`

**修复内容**:

```c
/* 从插件内容中提取元信息 */
static int extract_plugin_meta(const char *content, char *name, char *version,
                                char *author, char *description, char *icon, char *color) {
    /* 默认值 */
    strcpy(name, "未命名插件");
    strcpy(version, "1.0.0");
    strcpy(author, "未知");
    strcpy(description, "");
    strcpy(icon, "fa-puzzle-piece");
    strcpy(color, "from-blue-500 to-cyan-400");

    /* 查找 window.PLUGIN = { 或 PLUGIN = { 的位置 */
    const char *plugin_start = strstr(content, "window.PLUGIN");
    if (!plugin_start) {
        plugin_start = strstr(content, "PLUGIN");
    }

    /* 如果找不到PLUGIN定义，使用整个内容 */
    if (!plugin_start) {
        plugin_start = content;
    }

    /* 简单解析 name: 'xxx' 或 name: "xxx" */
    const char *patterns[][2] = {
        {"name:", name},
        {"version:", version},
        {"author:", author},
        {"description:", description},
        {"icon:", icon},
        {"color:", color}
    };

    for (int i = 0; i < 6; i++) {
        const char *p = strstr(plugin_start, patterns[i][0]);
        if (p) {
            p += strlen(patterns[i][0]);
            /* 跳过空白字符（包括空格、制表符、换行符） */
            while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
            if (*p == '\'' || *p == '"') {
                char quote = *p++;
                char *dst = (char *)patterns[i][1];
                int j = 0;
                /* 提取值，支持转义字符 */
                while (*p && *p != quote && j < 127) {
                    if (*p == '\\' && *(p+1)) {
                        /* 处理转义字符 */
                        p++;
                        if (*p == 'n') dst[j++] = '\n';
                        else if (*p == 't') dst[j++] = '\t';
                        else if (*p == 'r') dst[j++] = '\r';
                        else if (*p == '\\') dst[j++] = '\\';
                        else if (*p == '\'' || *p == '"') dst[j++] = *p;
                        else dst[j++] = *p;
                        p++;
                    } else {
                        dst[j++] = *p++;
                    }
                }
                dst[j] = '\0';
            }
        }
    }
    return 0;
}
```

**改进点**:
- ✅ 先查找 `window.PLUGIN` 或 `PLUGIN` 关键字，从正确位置开始解析
- ✅ 支持跳过换行符 `\n` 和 `\r`
- ✅ 添加转义字符处理（`\n`、`\t`、`\r`、`\\`、`\'`、`\"`）
- ✅ 更健壮的字符串提取逻辑

### 2. 创建测试插件

**文件**: `test-plugin.js`

创建了一个标准的测试插件，用于验证修复后的系统是否正常工作。该插件包含：
- 完整的元数据（name, version, author, description, icon, color）
- 使用 plugin-card 和 plugin-btn 组件的模板
- 响应式数据和方法
- 生命周期钩子

## 部署步骤

### 1. 编译后端

在交叉编译环境中执行：

```bash
cd src
make clean
make
```

这将生成 `build/ofono-server` 可执行文件。

### 2. 部署到设备

将编译好的 `ofono-server` 上传到设备的相应位置，并重启服务。

### 3. 测试插件

1. 使用 `test-plugin.js` 测试插件系统
2. 在插件商店中上传该测试插件
3. 验证插件名称、描述、图标是否正确显示
4. 点击"运行"按钮，验证插件UI是否正确渲染
5. 测试按钮功能是否正常

## 预期效果

修复后，插件应该：
- ✅ 正确显示插件名称（而不是"未命名插件"）
- ✅ 正确显示插件描述、版本、作者信息
- ✅ 正确显示自定义图标和颜色
- ✅ 按钮显示正确的文本（而不是"Button"）
- ✅ 插件功能正常运行

## 插件开发规范

确保插件按照以下格式编写：

```javascript
window.PLUGIN = {
  // 元数据（必填）
  name: '插件名称',
  version: '1.0.0',
  author: '作者名',
  description: '插件描述',

  // 元数据（可选）
  icon: 'fa-icon-name',
  color: 'from-blue-500 to-cyan-400',

  // 模板（必填）
  template: `
    <plugin-card title="标题" icon="icon-name">
      <plugin-btn type="primary" @click="methodName">
        按钮文本
      </plugin-btn>
    </plugin-card>
  `,

  // 数据（必填）
  data() {
    return {
      // 响应式数据
    }
  },

  // 方法（必填）
  methods: {
    async methodName() {
      // 方法实现
    }
  },

  // 生命周期（可选）
  mounted() {},
  destroyed() {}
}
```

## 注意事项

1. **元数据格式** - 确保 `name:`、`version:` 等字段后面有空格，值用单引号或双引号包裹
2. **模板内容** - 使用 `<plugin-btn>` 组件时，必须在标签内提供按钮文本
3. **方法绑定** - 使用 `@click="methodName"` 绑定方法，方法名不要加括号

## 相关文件

- `src/system/plugin.c` - 后端插件管理（已修复）
- `web/src/components/PluginStore.vue` - 前端插件商店
- `web/src/components/plugin/PluginBtn.vue` - 按钮组件
- `docs/插件开发指南2.0.md` - 插件开发文档
- `test-plugin.js` - 测试插件示例
