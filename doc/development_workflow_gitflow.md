# Development workflow

We are following the [shared repository model](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/about-collaborative-development-models) and this [Git branching model](https://nvie.com/posts/a-successful-git-branching-model/) at the current stage of the DAQ software development. Under the `shared repository model`, developers are granted push access to a single shared repository. Topic branches are created when changes need to be made by following the `Git branching model`.

## Repository Access

We use [two access roles](https://home.fnal.gov/~dingpf/repo_access_role.png) for each repo.

* Maintain: one or two developers have the "Maintainer" role to each repository;
* Write: a GitHub team of developers.

Teams are entities of the DUNE-DAQ project, thus they can be used across multiple repos.

A team is usually managed by DAQ working group leaders. Developers obtain write access by being added into a team. 

## Branches of DAQ repositories

<img src="https://nvie.com/img/git-model@2x.png" style="float:right" width="300" height="400">

* Required long-lived branches: `develop`, `master`;
* Default branch: `develop`;
* Short-lived branches:
  * feature branches
    * branch off from `develop`,
    * merge back to `develop`;
  * hotfix branches
    * branch off from `master`,
    * merge back to `develop` and `master`
  * release branches
    * branch off from `develop`,
    * merge back to `develop` and `master`
* Protected branch settings: protected branches are optional **for the moment**, repo maintainers can choose to "protect" `develop` and/or `master` branches. [Protections rules](https://docs.github.com/en/github/administering-a-repository/about-protected-branches#about-branch-protection-settings) can be further set up against those branches, such as require pull request reviews before merging, restrict who can push to matching branches, etc.

## Tags of DAQ repositories

We have two types of tags for DAQ repositories:

1. Version tags: 
 * made by repo maintainers
 * in the format of `vX.X.X` where X is a digit.
2. DAQ release tags: 
 * made by the software coordination team;
 * alias to a version tag;
 * in the format of `dunedaq-vX.X.X` where X is a digit.

## Development workflow (feature branches)

Developer is recommended to follow the following development workflow regardless of the amount of committed code change, e.g. either making a quick bugfix or adding a major feature. The workflow contains the following steps:

1. Create a GitHub issue in the repo describe the bugfix or proposed feature; [optional for non-significant bugfixes]
2. Create a topic branch; (`git checkout develop; git checkout -b dingpf/issue_12_feature_dev_demo`)
3. Make code development, commit, and push the topic branch to GitHub; (`git push -u origin dingpf/issue_12_feature_dev_demo`)
4. Create pull request to `develop` branch when the topic branch is ready to be reviewed and merged, link the issue created in step 1 to the pull request;
5. The pull request gets reviewed by other developers who can:
   * comment on the commits in the PR;
   * request changes;
   * approve pull requests and merge to `develop`;
   * delete the pull request branch once it's merged (optional), and close the linked issue.

## Tagging and releasing workflow (release branches)

Package maintainers are the primary developers who make version tags of a package. The following workflow should be used when doing so.

1. Check the state of the `develop` branch: verify all pull requests related to the planed release have been reviewed and merged;
2. Create a release branch; (`git checkout -b release-v2.2.0 develop`)
3. Make necessary changes such as bump versions in `CMakeLists.txt` in the release branch, commit and push;
4. Optional: (especially if protection rules are in place for the `master` branch)create a pull request of the release branch against both the `master` branch;
5. If not using step 4, merge the release branch to `master` (`git checkout master; git merge --no-ff release-v2.2.0 # always use the --no-ff option`), otherwise review&merge the pull requests (preferably done by other developers, protection rules can be set to enforce reviewing rules);
6. Tag the master branch; (`git tag -a v2.2.0 # use annotated tag`)
7. Merge the release branch to `develop` (if protection rules are in place for `develop`, one may need to create another pull request in this case); (`git checkout develop; git merge --no-ff release-v2.2.0`)
8. Optional: delete the release branch.

## Useful tips

* Using `#Issue_Number` in your commit message will make GitHub add links to the commit on the issue page;
* Use `user/repo#issue_number` to link issues in a different repo, e.g. `DUNE-DAQ/daq-cmake#1`;
* **Use with caution** Delete a git tag: `git tag -d v1.0.1` **Do not delete any tag which might be used by others**;
* Push the deletion of a tag to GitHub: `git push --delete origin v1.0.1` **Do not delete any tag which might be used by others**;
* Delete a remote branch: `git push origin --delete feature/branch_name` (not recommended; recommend to delete branches via GitHub web UI);
* List all tags in GitHub repo: `git ls-remote --tags origin`;
* Fetch all branches and tags: `git fetch --all --tags`.

## Screenshots of some examples

### Repository access

![repo-access](https://home.fnal.gov/~dingpf/repo_access_role.png)

### Branch settings

![branch-settings](https://home.fnal.gov/~dingpf/default-branch.png)

### Branch protection rules

![branch-protection-rules](https://home.fnal.gov/~dingpf/branch-protection-rule.png)

### Managing branches

![managing-branches](https://home.fnal.gov/~dingpf/managing-branches.png)

### View Network Graph

![network-graph](https://home.fnal.gov/~dingpf/network-graph.png)
