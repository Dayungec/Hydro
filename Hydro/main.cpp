#include <QCoreApplication>
#include "Base/autoptr.h"
#include "Base/FilePath.h"
#include "Base/LicenseEngine.h"
#include "Base/StringHelper.h"
#include "mcallback.h"
#include "task.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    LicenseEngine pEngine;
    //pEngine.StartUsing("E:/Qt/SGIS/Build/Assets");
    pEngine.StartUsing(FilePath::GetCurrentDir()+"/Assets");
    MCallBack callBack;
    vector<string>argstrs;
    for (int k = 1; k < argc; k++) {
        argstrs.push_back(argv[k]);
    }
    if (argstrs.size() < 1) {
        CallBackHelper::SendMessageA(&callBack,"Need Command Parameters");
        return 0;
    }

    string cmd = argstrs[0];
    StringHelper::MakeLower(cmd);
    if (cmd == "help") {
        if (argstrs.size() == 1) {
            Task::SendCmds(&callBack);
        }
        else {
            string taskName = argstrs[1];
            AutoPtr<Task>task(Task::CreateTask(taskName));
            if (task == NULL) {
                cout << "不能识别的命令:" << taskName << endl;
            }
            task->SendHelpString(&callBack);
        }
        return 0;
    }
    AutoPtr<Task>task(Task::CreateTask(cmd));
    if (task == NULL) {
        CallBackHelper::SendMessageA(&callBack,"Can not recognize "+ cmd);
        return 0;
    }
    if (!task->ReadParameters(argstrs, &callBack)) {
        CallBackHelper::SendMessageA(&callBack,"Command:" + cmd + " has invalid parameters.");
        CallBackHelper::SendMessageA(&callBack,"Command Parameters: ");
        task->SendHelpString(&callBack);
        return 0;
    }
    task->Execute(&callBack);
    return 0;
}
