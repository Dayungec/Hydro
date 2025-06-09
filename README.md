With windows, run the cmd command line in Hydro\Release\release.
Type Hydro help and the following command prompt will appear.

1. FillDem <demPath> <savingPath
    FillDem: FillDem, 
    <demPath>: dem file path, 
    <savingPath>: Saving path

2. FlowDirection <demPath> <savingPath>
    FlowDirection: direction of water flow, 
    <demPath>: dem file path, 
    <savingPath>: Saving path

3. FlowAccumulation <flowddir> <savingPath
    FlowAccumulation: Flow Accumulation Calculation, 
    <flowddir>: Direction of flow file path, 
    <savingPath>: Saving path

4. Sink <filldem> <lakeminradius> <savingPath>
    Sink: extracts the depression, 
    <filldem>: Path to the filled dem file, 
    <lakeminradius>: dpw, 
    <savingPath>: saving path

5. SpillLine <demPath> <filldemPath> <flowPath> <accuPath> <lakeMinRadius> <savingPath>
    SpillLine: spill line extraction, 
    <demPath>: dem file path, 
    <filldemPath>: dem file path after filldem, 
    <flowPath>: Direction of water flow file path, 
    <accuPath>: Accumulation file path, 
    <lakeMinRadius>: dpw, 
    <savingPath>: Saving path

6. Dig <demPath> <spilllinepath> <savingPath>
    Dig: Dig down, 
    <demPath>: dem file path, 
    <spilllinepath>: spill line file path, 
    <savingPath>: Saving path

7. SpillLineDig <demPath> <lakeMinRadius> <iters> <savingPath>
    SpillLineDig: Spill line based digging, batch processing (includes SpillLine and Dig functions), 
    <demPath> dem file path, 
    <lakeminradius>: dpw, 
    <iters>: number of iterations, 
    <savingPath>: saving path

Follow the prompts for each function to enter the correctly formatted commands to run the corresponding function code.
Run the commands in order 1-7.
