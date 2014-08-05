GDIPP
=====

GDIPP is a replacement of Windows’ [GDI](https://wikipedia.org/wiki/Graphics_Device_Interface) text rendering that brings beautiful, anti-aliased text similar to that on OS X and Linux.

The project originated as the relaunch of [GDI++](https://github.com/CoolOppo/GDI-PlusPlus). The source code of gdi++.dll project is too difficult to manage, so GDIPP is a new code-base. The goal of GDIPP project is to continue the development of gdi++, expand functionality, improve performance, provide better compatibility for the current and future Windows versions (potentially at the cost of removing compatibility for old Windows), create detailed and formal documentation, as well as tools to help using it.

GDIPP Project supports both 32-bit and 64-bit Windows 7/Vista. You can install 32-bit and 64-bit components separately. The 32-bit components only renders 32-bit applications, while the 64-bit components only renders 64-bit applications in 64-bit Windows. Both components share the same setting file. GDIPP also works correctly on Windows XP and 2003. However, due to the significant difference in the system architectures, the rendering may be less stable and compatible. [ezgdi](https://github.com/CoolOppo/ezgdi) is an alternative choice for a Windows text rendering replacement.

### GDIPP has accomplished the following goals:

- [x] Client-server model
- [x] FreeType renderer
- [x] GGO renderer
- [x] DirectWrite renderer
- [x] GDI painter
- [x] Fine-grain glyph/glyph run cache synchronization
- [x] Project renaming and moving
- [ ] Persistent caching (e.g. SQLite, Redis)
- [ ] Support for vertical text
- [ ] Support for right-to-left text
- [ ] GDIPP Preview
- [ ] Kernel-mode hooking (driver)
- [ ] Windows Mobile systems support

---

### Examples

The following pictures below illustrate the difference between the Windows font renderer ClearType and GDIPP. For each figure, the first picture is taken from ClearType while the second is taken from GDIPP. Pay attention to the anti-aliased effect GDIPP exhibits.

The examples use this text:

```
The quick brown fox jumps over the lazy dog 1234567890
敏捷的棕毛狐狸从懒狗身上跃过
すばしっこい茶色の狐はのろまな犬を飛び越える
날쌘 갈색 여우가 게으른 개를 뛰어넘는다
```

#### Segoe UI, and font linking to Simplified Chinese font Microsoft YaHei.

![](http://i.imgur.com/HQABEpA.png)

![](http://i.imgur.com/aoV83LX.png)

#### Microsoft YaHei - Since it has contains all characters, no font linking happens.

![](http://i.imgur.com/7qHDvMK.png)

![](http://i.imgur.com/K5jH6d4.png)

#### MS Mincho and default font linking setting.

![](http://i.imgur.com/kpM9Wmo.png)

![](http://i.imgur.com/PR1fSfF.png)
