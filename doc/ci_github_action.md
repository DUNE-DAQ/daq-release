# Instructions on setting up CI using GitHub Action

This is a placeholder for instructions on setting up CI using GitHub action.

Here is a [presentation](https://indico.fnal.gov/event/45266/contributions/195659/attachments/133754/165107/DUNE_DAQ_SW_2020831.pdf) in SW Coordination Meeting on CI prototype

## Phase one

1. Create Docker image for running CI jobs;
2. Using runner nodes on cloud server provided by GitHub Action, CI jobs will be run inside containers;
3. Configure CI projects associated to each repo, this is how GitHub Action tracks commits/PRs and triggers CI;
4. Workflow configuration yaml files live with each repo;
5. CIs for the following repos on the `develop` branch:
	* `daq-buildtools`;
	* `appfwk`;
	* `listrev`;
	* `ddpdemo`;
5. Setting up proper notification; auto triggering for PRs, and commits to the `develop` branch.


## Phase two

The major work of Phase Two is to set up customized runner nodes running CentOS 7/8 and then test CI jobs both with and without containers.


