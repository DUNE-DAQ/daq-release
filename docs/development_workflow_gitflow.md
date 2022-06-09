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
 * in the format of `vX.Y.Z` where `X`, `Y` and `Z` is a digit for `MAJOR, MINOR, PATCH` version respectively;
 * to a minimum, if `X` is not advanced in a newer DAQ release, and a new tag is needed, the minor version `Y` should be advanced.
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
   * delete the pull request branch once it's merged (enabled by GitHub by default), and close the linked issue (automatically done by GitHub for the linked issues).

💡 If the targeted branch of the pull request has advanced, please do the following to bring the feature branch in sync before merging the PR:
1. Switch to the targeted branch, and do a `git pull` to make sure it stays in sync with the remote;
2. Switch back to the feature branch of the PR, merge the targeted branch into it, e.g. `git merge --no--ff <targeted branch name>`;
3. Push the merge to remote, and continue with the PR review/merge process.

:red_circle: Please don't use `git rebase` or `git push --force`. It will likely bring unexpected consequences.


## Tagging and releasing workflow (release branches)

Package maintainers are the primary developers who make version tags of a package. The following workflow should be used when doing so.

1. Check the state of the `develop` branch: verify all pull requests related to the planed release have been reviewed and merged;
2. Create a release preparation branch (e.g. `git checkout -b prep-release/dunedaq-v2.2.0`);
3. Make necessary changes such as bump versions in `CMakeLists.txt` in the release branch, commit and push;
4. Create new tag on the release preparation branch;
4. Create a pull request of the release preparation branch against the `develop` branch;
5. Review and merge the pull requests. This can be done by another developer, or use the "auto approval" GitHub Action to add a dummy approval to the PR.

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

![repo-access](https://i.imgur.com/ddLJeif.png)

### Branch settings

![branch-settings](https://i.imgur.com/WbBJB86.png)

### Branch protection rules

![branch-protection-rules](https://i.imgur.com/NMp0vMU.png)

### Managing branches

![managing-branches](https://i.imgur.com/d25W5er.png)

### View Network Graph

![network-graph](https://i.imgur.com/ogmjKYr.png)
