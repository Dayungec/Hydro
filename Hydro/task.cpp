#include "task.h"
#include "Base/StringHelper.h"
#include "Base/FilePath.h"
#include "Datasources/filerasterdataset.h"
#include "SpatialAnalyst/samodel.h"
#include "SpatialAnalyst/hydroanalyst.h"


REGISTER(FillDemTask)
REGISTER(FlowDirectionTask)
REGISTER(FlowAccumulationTask)
REGISTER(SinkTask)
REGISTER(SpillLineTask)
REGISTER(DigTask)
REGISTER(SpillLineDigTask)

Task::Task()
{

}

Task::~Task() {

}
vector<AutoPtr<Task>>Task::CreateStockTasks() {
    vector<AutoPtr<Task>>tasks=ClassFactory::getObjects<Task>();
    return tasks;
}

AutoPtr<Task>Task::CreateTask(string taskName) {
    StringHelper::MakeLower(taskName);
    vector<AutoPtr<Task>>tasks = CreateStockTasks();
    AutoPtr<Task>task = nullptr;
    for (int k = tasks.size() - 1; k >= 0; k--) {
        string name = tasks[k]->GetTaskName();
        StringHelper::MakeLower(name);
        if (name == taskName) {
            task= tasks[k];
            tasks.erase(std::begin(tasks) + k);
            break;
        }
    }
    return task;
}

void Task::SendCmds(CallBack*callBack) {
    vector<AutoPtr<Task>>tasks = CreateStockTasks();
    for (int k = 0; k < tasks.size();k++) {
        tasks[k]->SendHelpString(callBack);
    }
}

bool Task::GetBoolValue(string arg) {
    string sarg = arg;
    StringHelper::MakeLower(sarg);
    if ((sarg == "true") || (sarg == "t") || (sarg == "yes") || (sarg == "y")) return true;
    return false;
}

void Task::Sleep(int miniseconds){
    DateTime ft;
    while(true){
        if((DateTime::Now()-ft).TotalSeconds()*1000>=miniseconds) return;
    }
}

FillDemTask::FillDemTask(){

}

FillDemTask::~FillDemTask(){

}

string FillDemTask::GetTypeName(){
    return "FillDemTask";
}

string FillDemTask::GetTaskName(){
    return "FillDem";
}


bool FillDemTask::ReadParameters(vector<string>argstrs, CallBack*callBack){
    int argSize = argstrs.size();
    if (argSize < 3) {
        CallBackHelper::SendMessage(callBack,"Invalid Parameters.");
        return false;
    }
    demPath=argstrs[1];
    savingPath=argstrs[2];
    return true;
}

void FillDemTask::Execute(CallBack*callBack){
    AutoPtr<FileRasterDataset>demds(new FileRasterDataset());
    if(!demds->OpenFromFile(demPath)){
        CallBackHelper::SendMessage(callBack,"Invalid dem path.");
        return;
    }
    AutoPtr<SAEnvironment>pEnvi(new SAEnvironment());
    pEnvi->CreateFromRasterBand(demds->GetRasterBand(0).get());
    AutoPtr<HydroDemFill>demfill(new HydroDemFill());
    demfill->SetEnvironment(pEnvi);
    AutoPtr<FileRasterTarget>target(new FileRasterTarget());
    target->PathName()=savingPath;
    demfill->Execute(demds->GetRasterBand(0).get(),target.get(),callBack);
    CallBackHelper::SendMessage(callBack,"Dem fill Finished");
}

void FillDemTask::SendHelpString(CallBack*callBack) {
    callBack->SendMessage(GetTaskName()+" <demPath> <savingPath>");
}

FlowDirectionTask::FlowDirectionTask(){

}

FlowDirectionTask::~FlowDirectionTask(){

}

string FlowDirectionTask::GetTypeName(){
    return "FlowDirectionTask";
}

string FlowDirectionTask::GetTaskName(){
    return "FlowDirection";
}


bool FlowDirectionTask::ReadParameters(vector<string>argstrs, CallBack*callBack){
    int argSize = argstrs.size();
    if (argSize < 3) {
        CallBackHelper::SendMessage(callBack,"Invalid Parameters.");
        return false;
    }
    demPath=argstrs[1];
    savingPath=argstrs[2];
    return true;
}

void FlowDirectionTask::Execute(CallBack*callBack){
    AutoPtr<FileRasterDataset>demds(new FileRasterDataset());
    if(!demds->OpenFromFile(demPath)){
        CallBackHelper::SendMessage(callBack,"Invalid filled dem path.");
        return;
    }
    AutoPtr<SAEnvironment>pEnvi(new SAEnvironment());
    pEnvi->CreateFromRasterBand(demds->GetRasterBand(0).get());
    AutoPtr<HydroFlowDirection>flow(new HydroFlowDirection());
    flow->SetEnvironment(pEnvi);
    AutoPtr<FileRasterTarget>target(new FileRasterTarget());
    target->PathName()=savingPath;
    flow->Execute(demds->GetRasterBand(0).get(),target.get(),callBack);
    CallBackHelper::SendMessage(callBack,"Dem flow direction Finished");
}


void FlowDirectionTask::SendHelpString(CallBack*callBack) {
    callBack->SendMessage(GetTaskName()+" <demPath> <savingPath>");
}

FlowAccumulationTask::FlowAccumulationTask(){

}

FlowAccumulationTask::~FlowAccumulationTask(){

}

string FlowAccumulationTask::GetTypeName(){
    return "FlowAccumulationTask";
}

string FlowAccumulationTask::GetTaskName(){
    return "FlowAccumulation";
}

bool FlowAccumulationTask::ReadParameters(vector<string>argstrs, CallBack*callBack){
    int argSize = argstrs.size();
    if (argSize < 3) {
        CallBackHelper::SendMessage(callBack,"Invalid Parameters.");
        return false;
    }
    demPath=argstrs[1];
    savingPath=argstrs[2];
    return true;
}

void FlowAccumulationTask::Execute(CallBack*callBack){
    AutoPtr<FileRasterDataset>flowds(new FileRasterDataset());
    if(!flowds->OpenFromFile(demPath)){
        CallBackHelper::SendMessage(callBack,"Invalid flow dir path.");
        return;
    }
    AutoPtr<SAEnvironment>pEnvi(new SAEnvironment());
    pEnvi->CreateFromRasterBand(flowds->GetRasterBand(0).get());
    AutoPtr<HydroFlowAccumulation>accu(new HydroFlowAccumulation());
    accu->SetEnvironment(pEnvi);
    AutoPtr<FileRasterTarget>target(new FileRasterTarget());
    target->PathName()=savingPath;
    accu->Execute(flowds->GetRasterBand(0).get(),nullptr,target.get(),callBack);
    CallBackHelper::SendMessage(callBack,"Dem flow accumalation Finished");
}


void FlowAccumulationTask::SendHelpString(CallBack*callBack) {
    callBack->SendMessage(GetTaskName()+" <flowddir> <savingPath>");
}


SinkTask::SinkTask(){

}

SinkTask::~SinkTask(){

}

string SinkTask::GetTypeName(){
    return "SinkTask";
}

string SinkTask::GetTaskName(){
    return "Sink";
}

bool SinkTask::ReadParameters(vector<string>argstrs, CallBack*callBack){
    int argSize = argstrs.size();
    if (argSize < 4) {
        CallBackHelper::SendMessage(callBack,"Invalid Parameters.");
        return false;
    }
    demPath=argstrs[1];
    lakeMinRadius=atoi(argstrs[2].c_str());
    if(lakeMinRadius<1) lakeMinRadius=1;
    savingPath=argstrs[3];
    return true;
}

void SinkTask::Execute(CallBack*callBack){
    AutoPtr<FileRasterDataset>fillds(new FileRasterDataset());
    if(!fillds->OpenFromFile(demPath)){
        CallBackHelper::SendMessage(callBack,"Invalid filled dem path.");
        return;
    }
    AutoPtr<SAEnvironment>pEnvi(new SAEnvironment());
    pEnvi->CreateFromRasterBand(fillds->GetRasterBand(0).get());
    AutoPtr<HydroLake>sinks(new HydroLake());
    sinks->SetEnvironment(pEnvi);
    AutoPtr<FileRasterTarget>target(new FileRasterTarget());
    target->PathName()=savingPath;
    sinks->Execute(fillds->GetRasterBand(0).get(),lakeMinRadius,false,target.get(),callBack);
    CallBackHelper::SendMessage(callBack,"extract Sinks Finished");
}


void SinkTask::SendHelpString(CallBack*callBack) {
    callBack->SendMessage(GetTaskName()+" <filldem> <lakeminradius> <savingPath>");
}

SpillLineTask::SpillLineTask(){

}

SpillLineTask::~SpillLineTask(){

}

string SpillLineTask::GetTypeName(){
    return "SpillLineTask";
}

string SpillLineTask::GetTaskName(){
    return "SpillLine";
}


bool SpillLineTask::ReadParameters(vector<string>argstrs, CallBack*callBack){
    int argSize = argstrs.size();
    if (argSize < 7) {
        CallBackHelper::SendMessage(callBack,"Invalid Parameters.");
        return false;
    }
    demPath=argstrs[1];
    filldemPath=argstrs[2];
    flowPath=argstrs[3];
    accuPath=argstrs[4];
    lakeMinRadius=atoi(argstrs[5].c_str());
    if(lakeMinRadius<1) lakeMinRadius=1;
    savingPath=argstrs[6];
    return true;
}

void SpillLineTask::Execute(CallBack*callBack){
    AutoPtr<FileRasterDataset>demds(new FileRasterDataset());
    if(!demds->OpenFromFile(demPath)){
        CallBackHelper::SendMessage(callBack,"Invalid dem path.");
        return;
    }
    AutoPtr<FileRasterDataset>filldemds(new FileRasterDataset());
    if(!filldemds->OpenFromFile(filldemPath)){
        CallBackHelper::SendMessage(callBack,"Invalid filled dem path.");
        return;
    }
    AutoPtr<FileRasterDataset>flowdemds(new FileRasterDataset());
    if(!flowdemds->OpenFromFile(flowPath)){
        CallBackHelper::SendMessage(callBack,"Invalid flow path.");
        return;
    }
    AutoPtr<FileRasterDataset>accuds(new FileRasterDataset());
    if(!accuds->OpenFromFile(accuPath)){
        CallBackHelper::SendMessage(callBack,"Invalid accu path.");
        return;
    }
    AutoPtr<SAEnvironment>pEnvi(new SAEnvironment());
    pEnvi->CreateFromRasterBand(demds->GetRasterBand(0).get());
    AutoPtr<HydroLakePouringLine>hlpl(new HydroLakePouringLine());
    hlpl->SetEnvironment(pEnvi);
    AutoPtr<ShapefileTarget>target(new ShapefileTarget());
    target->PathName()=savingPath;
    hlpl->Execute(demds->GetRasterBand(0).get(),filldemds->GetRasterBand(0).get(),flowdemds->GetRasterBand(0).get(),accuds->GetRasterBand(0).get(),lakeMinRadius,target.get(),callBack);
    CallBackHelper::SendMessage(callBack,"HydroLakePouringLine Finished");
}

void SpillLineTask::SendHelpString(CallBack*callBack) {
    callBack->SendMessage(GetTaskName()+" <demPath> <filldemPath> <flowPath> <accuPath> <lakeMinRadius> <savingPath>");
}

DigTask::DigTask(){

}

DigTask::~DigTask(){

}

string DigTask::GetTypeName(){
    return "DigTask";
}

string DigTask::GetTaskName(){
    return "Dig";
}


bool DigTask::ReadParameters(vector<string>argstrs, CallBack*callBack){
    int argSize = argstrs.size();
    if (argSize < 4) {
        CallBackHelper::SendMessage(callBack,"Invalid Parameters.");
        return false;
    }
    demPath=argstrs[1];
    spillLinePath=argstrs[2];
    savingPath=argstrs[3];
    return true;
}

void DigTask::Execute(CallBack*callBack){
    AutoPtr<FileRasterDataset>demds(new FileRasterDataset());
    if(!demds->OpenFromFile(demPath)){
        CallBackHelper::SendMessage(callBack,"Invalid dem path.");
        return;
    }
    AutoPtr<ShapefileFeatureClass>spillfc(new ShapefileFeatureClass());
    if(!spillfc->OpenFromFile(spillLinePath)){
        CallBackHelper::SendMessage(callBack,"Invalid spill line path.");
        return;
    }
    AutoPtr<SAEnvironment>pEnvi(new SAEnvironment());
    pEnvi->CreateFromRasterBand(demds->GetRasterBand(0).get());
    AutoPtr<HydroDig>hlpl(new HydroDig());
    hlpl->SetEnvironment(pEnvi);
    AutoPtr<FileRasterTarget>target(new FileRasterTarget());
    target->PathName()=savingPath;
    hlpl->Execute(demds->GetRasterBand(0).get(),spillfc.get(),target.get(),callBack);
    CallBackHelper::SendMessage(callBack,"HydroLakePouringLine Finished");
}

void DigTask::SendHelpString(CallBack*callBack) {
    callBack->SendMessage(GetTaskName()+" <demPath> <spilllinepath> <savingPath>");
}



SpillLineDigTask::SpillLineDigTask(){

}

SpillLineDigTask::~SpillLineDigTask(){

}

string SpillLineDigTask::GetTypeName(){
    return "SpillLineDigTask";
}

string SpillLineDigTask::GetTaskName(){
    return "SpillLineDig";
}


bool SpillLineDigTask::ReadParameters(vector<string>argstrs, CallBack*callBack){
    int argSize = argstrs.size();
    if (argSize < 5) {
        CallBackHelper::SendMessage(callBack,"Invalid Parameters.");
        return false;
    }
    demPath=argstrs[1];
    lakeMinRadius=atoi(argstrs[2].c_str());
    if(lakeMinRadius<1) lakeMinRadius=1;
    iters=atoi(argstrs[3].c_str());
    if(iters<1) iters=1;
    savingPath=argstrs[4];
    return true;
}

void SpillLineDigTask::Execute(CallBack*callBack){
    AutoPtr<FileRasterDataset>demds(new FileRasterDataset());
    if(!demds->OpenFromFile(demPath)){
        CallBackHelper::SendMessage(callBack,"Invalid dem path.");
        return;
    }
    AutoPtr<SAEnvironment>pEnvi(new SAEnvironment());
    pEnvi->CreateFromRasterBand(demds->GetRasterBand(0).get());
    AutoPtr<HydroLakePouringLineDig>hlpl(new HydroLakePouringLineDig());
    hlpl->SetEnvironment(pEnvi);
    AutoPtr<FileRasterTarget>target(new FileRasterTarget());
    target->PathName()=savingPath;
    hlpl->Execute(demds->GetRasterBand(0).get(),lakeMinRadius,false,iters,target.get(),callBack);
    CallBackHelper::SendMessage(callBack,"HydroSpillLine Dig Finished");
}

void SpillLineDigTask::SendHelpString(CallBack*callBack) {
    callBack->SendMessage(GetTaskName()+" <demPath> <lakeMinRadius> <iters> <savingPath>");
}
