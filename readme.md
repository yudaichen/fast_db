# ubuntu24 安装：

## clang安装

https://apt.llvm.org/


编辑 APT 源列表。你可以使用文本编辑器（如 nano 或 vim）来编辑 /etc/apt/sources.list 文件，或者在 /etc/apt/sources.list.d/ 目录中创建一个新的文件。例如，使用 nano 创建一个名为 llvm.list 的文件：

    sudo nano /etc/apt/sources.list.d/llvm.list
添加以下内容到文件中：

    # Noble (24.04) - Last update : Fri, 20 Dec 2024 01:33:14 UTC / Revision: 20241219100923+5f096fd22160
    deb http://apt.llvm.org/noble/ llvm-toolchain-noble main
    deb-src http://apt.llvm.org/noble/ llvm-toolchain-noble main
    # 18
    deb http://apt.llvm.org/noble/ llvm-toolchain-noble-18 main
    deb-src http://apt.llvm.org/noble/ llvm-toolchain-noble-18 main
    # 19
    deb http://apt.llvm.org/noble/ llvm-toolchain-noble-19 main
    deb-src http://apt.llvm.org/noble/ llvm-toolchain-noble-19 main
    保存并退出编辑器。对于 nano，你可以按 CTRL + O 保存，然后按 CTRL + X 退出。

更新 APT 包索引：

    sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 15CF4D18AF4F7421
    sudo apt update
    sudo apt install clang-19 libclang-common-19-dev

## gcc-14 g++-14
    
    apt install gcc-14 g++-14
    sudo update-alternatives —install /usr/bin/gcc gcc /usr/bin/gcc-14 14
    sudo update-alternatives —install /usr/bin/g++ g++ /usr/bin/g++-14 14

