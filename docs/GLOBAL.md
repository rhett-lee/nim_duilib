# Global（全局样式）

Global 全局样式提供了通用的样式列表，避免在多个不同的 XML 中出现相同的描述而产生冗余的代码和消耗开发人员在界面设置上的时间。

在调用 GlobalManager::Startup 方法后，会根据设定的皮肤资源路径下查找 `global.xml` 作为全局的样式资源。在现有的 samples 示例代码中，
包含了一些预设的全局样式，如字体、颜色和一些通用样式。

## 字体（Font）

如果你想添加一个字体，则在 `global.xml` 中添加如下代码，程序启动后会加载所有字体列表到缓存中，以 ID 作为区分。

```xml
<!-- name 代表字体名称，size 代表字体大小，bold 代表是否粗体，underline 代表是否包含下划线 -->
<Font id="system_12" name="system" size="10" bold="true" underline="true"/>
```

当需要使用时，指定字体的编号即可。比如你希望一个 Button 按钮使用 ID 为 system_12 的字体，可以这样写：

```xml
<Button text="Hello Button" font="system_12"/>
```

### Font 所有可用属性

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| id | | string | 字体ID |
| name | | string | 字体在系统中的名称 |
| size | 12 | int | 字体大小 |
| bold | false | bool | 是否粗体 |
| underline | false | bool | 是否下划线 |
| strikeout | false | bool | 是否删除线 |
| italic | false | bool | 是否斜体 |
| default | false | bool | 是否是默认字体，如果未给控件指定字体，则使用该字体 |

字体属性解析相关代码，参见`WindowBuilder::ParseFontXmlNode`函数
## 颜色（TextColor）

你可以添加常用的颜色到 `global.xml` 中，如下所示：

```xml
<!-- name 是颜色的名称，value 是颜色的具体数值 -->
<TextColor name="default_font_color" value="#ff333333"/>
```

这样当你需要使用这个颜色给一个 Label 设置文字颜色时，可以这样写：

```xml
<Label text="Hello Label" normal_text_color="default_font_color"/>
```

### TextColor 所有可用属性

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| name | | string | 颜色名称 |
| value | | string | 颜色值如 0xFFFFFFFF |

## 通用样式（Class）

通用样式可以让我们预设一些经常使用到的样式集合，比如一个高度为 34，宽度自动拉伸、使用 caption.png 作为背景的标题栏。
又或者一个宽度为 80，高度为 30 的通用样式按钮等等。我们都可以通过通用样式来解决。以下示例演示了一个通用样式按钮：

```xml
<!-- name 是通用样式的名称，value 是通用样式的具体样式值 -->
<Class name="btn_global_blue_80x30" value="font=&quot;7&quot; normal_text_color=&quot;white&quot; normal_image=&quot;file='../public/button/btn_global_blue_80x30_normal.png'&quot; hot_image=&quot;file='../public/button/btn_global_blue_80x30_hovered.png'&quot; pushed_image=&quot;file='../public/button/btn_global_blue_80x30_pushed.png'&quot; disabled_image=&quot;file='../public/button/btn_global_blue_80x30_normal.png' fade='80'&quot;"/>
```

以上代码定义了一个按钮通用样式，命名为 `btn_global_blue_80x30`，使用字体编号为7，普通样式下字体颜色为 `white`，
并分别设置了普通状态、焦点状态和按下状态不同的背景图片，最后还启用了动画效果。当我们需要给一个按钮应用这个通用样式时，可以这样写：

```xml
<Button class="btn_global_blue_80x30" text="classic button"/>
```

要注意的是，**`class` 属性必须在所有属性最前面**，当你需要覆盖一个通用样式中指定过的属性时，只需要在 `class` 属性之后再重新定义这个属性就可以了。比如我希望我的按钮不使用通用样式的字体样色，可以这样写：

```xml
<Button class="btn_global_blue_80x30" font="2" text="classic button"/>
```

### Class 所有可用属性

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| name | | string | 通用样式名称 |
| 任何自定义名称 | | string | 通用样式的值，必须经过XML转义 |

## 全局资源管理相关的接口

| 类名称 | 关联头文件| 用途 |
| :--- | :--- | :--- |
| GlobalManager | [duilib/Core/GlobalManager.h](../duilib/Core/GlobalManager.h) | 全局属性管理工具类，用于管理一些全局属性的工具类，包含全局样式（global.xml）和语言设置等 |
| IRenderFactory | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | 渲染接口的管理类，渲染接口管理，用于创建Font、Pen、Brush、Path、Matrix、Bitmap、Render等渲染实现对象 |
| FontManager | [duilib/Core/FontManager.h](../duilib/Core/FontManager.h) | 字体的管理类 |
| ImageManager | [duilib/Core/ImageManager.h](../duilib/Core/ImageManager.h) | 图片的管理类 |
| IconManager | [duilib/Core/IconManager.h](../duilib/Core/IconManager.h) | HICON句柄管理器 |
| ZipManager | [duilib/Core/ZipManager.h](../duilib/Core/ZipManager.h) | ZIP压缩包管理器 |
| DpiManager | [duilib/Core/DpiManager.h](../duilib/Core/DpiManager.h) | DPI管理器，用于支持DPI自适应等功能 |
| TimerManager | [duilib/Core/TimerManager.h](../duilib/Core/TimerManager.h) | 定时器管理器 |
| LangManager | [duilib/Core/LangManager.h](../duilib/Core/LangManager.h) | 多语言支持管理器 |
