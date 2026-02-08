<<<<<<< HEAD
#FF_1070
# FF项目编译、文件拷贝及升级镜像制作全流程指南
本文档用于指导完成FF项目的编译、文件拷贝及升级镜像制作全流程，通过自动化脚本完成镜像打包、文件同步及外部SDK编译，最终生成设备可使用的升级镜像文件。

## 前置条件
执行编译操作前，请务必确认满足以下所有条件：

### 1. 环境依赖
已安装以下基础编译/打包工具：
- cmake
- make
- squashfs-tools（需确保`mksquashfs`工具可用）
- tar

### 2. 路径检查
确认以下路径均存在且符合要求（路径不一致时需修改`make_image.sh`中的硬编码路径）：
1. 外部SDK路径：`/home/sl/share/FF_Indoor/AK37E_SDK_V1.05`（需包含可执行的`auto_build.sh`脚本）
2. 项目根路径：`/home/zio/Share/FF_Indoor/EP_1070`（需为当前项目实际根目录）
3. 目标拷贝目录：`../../AK37E_SDK_V1.05/rootfs/resource/app/app`（相对路径基于`make_image.sh`执行位置）

### 3. 文件依赖
项目根目录下需存在以下目录及文件：
- `build`目录：内含`CMakeLists.txt`编译配置文件
- `res`目录：内含`rom.bin`、`sat_leo.ttf`、`persian.ttf`文件及`rings`目录

## 编译步骤
### 1. 基础编译
执行以下命令完成项目基础编译：
```bash
# 进入项目根目录（请替换为实际路径）
cd /home/zio/Share/FF_Indoor/EP_1070
# 执行自动化编译（触发Makefile及后续镜像打包流程）
make
```

### 2. 清理编译产物
若需清理编译生成的产物，执行以下命令：
```bash
# 进入项目根目录后执行清理
make clean
```

## 补充说明
- 整个流程的核心逻辑：`Makefile`自动化触发编译 → 调用`make_image.sh`脚本 → 完成镜像打包、文件同步、外部SDK编译 → 生成设备升级镜像。
- 若编译过程中提示路径不存在/脚本不可执行，优先检查「前置条件」中的路径和文件依赖是否满足。

### 总结
1. 编译前需确认环境、路径、文件三类前置依赖全部满足，路径不一致时需修改`make_image.sh`硬编码路径；
2. 核心编译命令为进入项目根目录后执行`make`，清理产物执行`make clean`；
3. 流程核心是通过Makefile+shell脚本自动化完成编译到镜像生成的全流程。
=======
# FF



## Getting started

To make it easy for you to get started with GitLab, here's a list of recommended next steps.

Already a pro? Just edit this README.md and make it your own. Want to make it easy? [Use the template at the bottom](#editing-this-readme)!

## Add your files

- [ ] [Create](https://docs.gitlab.com/ee/user/project/repository/web_editor.html#create-a-file) or [upload](https://docs.gitlab.com/ee/user/project/repository/web_editor.html#upload-a-file) files
- [ ] [Add files using the command line](https://docs.gitlab.com/ee/gitlab-basics/add-file.html#add-a-file-using-the-command-line) or push an existing Git repository with the following command:

```
cd existing_repo
git remote add origin http://127.0.0.1/sat_songling/ff.git
git branch -M main
git push -uf origin main
```

## Integrate with your tools

- [ ] [Set up project integrations](http://127.0.0.1/sat_songling/ff/-/settings/integrations)

## Collaborate with your team

- [ ] [Invite team members and collaborators](https://docs.gitlab.com/ee/user/project/members/)
- [ ] [Create a new merge request](https://docs.gitlab.com/ee/user/project/merge_requests/creating_merge_requests.html)
- [ ] [Automatically close issues from merge requests](https://docs.gitlab.com/ee/user/project/issues/managing_issues.html#closing-issues-automatically)
- [ ] [Enable merge request approvals](https://docs.gitlab.com/ee/user/project/merge_requests/approvals/)
- [ ] [Set auto-merge](https://docs.gitlab.com/ee/user/project/merge_requests/merge_when_pipeline_succeeds.html)

## Test and Deploy

Use the built-in continuous integration in GitLab.

- [ ] [Get started with GitLab CI/CD](https://docs.gitlab.com/ee/ci/quick_start/index.html)
- [ ] [Analyze your code for known vulnerabilities with Static Application Security Testing (SAST)](https://docs.gitlab.com/ee/user/application_security/sast/)
- [ ] [Deploy to Kubernetes, Amazon EC2, or Amazon ECS using Auto Deploy](https://docs.gitlab.com/ee/topics/autodevops/requirements.html)
- [ ] [Use pull-based deployments for improved Kubernetes management](https://docs.gitlab.com/ee/user/clusters/agent/)
- [ ] [Set up protected environments](https://docs.gitlab.com/ee/ci/environments/protected_environments.html)

***

# Editing this README

When you're ready to make this README your own, just edit this file and use the handy template below (or feel free to structure it however you want - this is just a starting point!). Thanks to [makeareadme.com](https://www.makeareadme.com/) for this template.

## Suggestions for a good README

Every project is different, so consider which of these sections apply to yours. The sections used in the template are suggestions for most open source projects. Also keep in mind that while a README can be too long and detailed, too long is better than too short. If you think your README is too long, consider utilizing another form of documentation rather than cutting out information.

## Name
Choose a self-explaining name for your project.

## Description
Let people know what your project can do specifically. Provide context and add a link to any reference visitors might be unfamiliar with. A list of Features or a Background subsection can also be added here. If there are alternatives to your project, this is a good place to list differentiating factors.

## Badges
On some READMEs, you may see small images that convey metadata, such as whether or not all the tests are passing for the project. You can use Shields to add some to your README. Many services also have instructions for adding a badge.

## Visuals
Depending on what you are making, it can be a good idea to include screenshots or even a video (you'll frequently see GIFs rather than actual videos). Tools like ttygif can help, but check out Asciinema for a more sophisticated method.

## Installation
Within a particular ecosystem, there may be a common way of installing things, such as using Yarn, NuGet, or Homebrew. However, consider the possibility that whoever is reading your README is a novice and would like more guidance. Listing specific steps helps remove ambiguity and gets people to using your project as quickly as possible. If it only runs in a specific context like a particular programming language version or operating system or has dependencies that have to be installed manually, also add a Requirements subsection.

## Usage
Use examples liberally, and show the expected output if you can. It's helpful to have inline the smallest example of usage that you can demonstrate, while providing links to more sophisticated examples if they are too long to reasonably include in the README.

## Support
Tell people where they can go to for help. It can be any combination of an issue tracker, a chat room, an email address, etc.

## Roadmap
If you have ideas for releases in the future, it is a good idea to list them in the README.

## Contributing
State if you are open to contributions and what your requirements are for accepting them.

For people who want to make changes to your project, it's helpful to have some documentation on how to get started. Perhaps there is a script that they should run or some environment variables that they need to set. Make these steps explicit. These instructions could also be useful to your future self.

You can also document commands to lint the code or run tests. These steps help to ensure high code quality and reduce the likelihood that the changes inadvertently break something. Having instructions for running tests is especially helpful if it requires external setup, such as starting a Selenium server for testing in a browser.

## Authors and acknowledgment
Show your appreciation to those who have contributed to the project.

## License
For open source projects, say how it is licensed.

## Project status
If you have run out of energy or time for your project, put a note at the top of the README saying that development has slowed down or stopped completely. Someone may choose to fork your project or volunteer to step in as a maintainer or owner, allowing your project to keep going. You can also make an explicit request for maintainers.
>>>>>>> 9d0b02b6f03a7a4335523179cd086b80dcea2d3a
