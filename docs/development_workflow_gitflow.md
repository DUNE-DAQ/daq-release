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

* Required long-lived branches: `develop` (default branch of each repository);
* Release preparation branches `prep-release/dunedaq-v*` (i.e. `prep-release/dunedaq-v3.1.0`)
    * branch off from the tag created on the `develop` branch at the time of tag collection during a release cycle;
    * can be updated via PRs with at lease one approval review before release cut-off time, 
    * in general, should be merged to `develop` after release cut-off.
* Patch branches `patch/dunedaq-v*` (i.e. `patch/dunedaq-v3.0.x`)
    * branch off from a tagged version used in the release where the fixes apply;
    * merge back to `develop` if the fixes apply and should be used by the future releases

* Protected branch settings:
    * `develop`: "Require a pull request before merging" -- all commits must be made to a non-protected branch and submitted via a pull request before they can be merged;
    * `patch` and `prep-release` branches: in addition to the protection rules used in the `develop` branch, pull request to these branches require at least one approval review before they can be merged.

## Tags of DAQ repositories

We have two types of tags for DAQ repositories:

* Version tags: 
    * made by repo maintainers
    * in the format of `vX.Y.Z` where `X`, `Y` and `Z` is a digit for `MAJOR, MINOR, PATCH` version respectively;
    * at a minimum, if `X` is not advanced in a newer DAQ release, and a new tag is needed, the minor version `Y` should be advanced.
* DAQ release tags: 
    * made by the software coordination team;
    * alias to a version tag;
    * in the format of `dunedaq-vX.X.X` where X is a digit.

## Release cycle 

We have adopted a three-phase release cycle:
1. Phase-1, active development period.
2. Phase-2, testing period.
3. Phase-3, post release (patch release) period.

### Phase 1 - Active Development Period
 
In this period, developers make frequent updates to the `develop` branch via pull requests. The workflow will be like the following:

1. Create a GitHub issue in the repo describe the code change. This is optional for small changes.
2. Create a topic branch; (`git checkout develop; git checkout -b dingpf/issue_12_feature_dev_demo`)
3. Make code development, commit, and push the topic branch to GitHub; (`git push -u origin dingpf/issue_12_feature_dev_demo`)
4. Create a pull request to the `develop` branch and link the issue to the pull request if one was created in step 1;
5. Technically, the pull request can be merged without reviews. But it's highly recommended the author request reviews from other developers if the code change is significant.

💡 If the targeted branch of the pull request has advanced, please do the following to bring the feature branch in sync before merging the PR:
1. Switch to the targeted branch, and do a `git pull` to make sure it stays in sync with the remote;
2. Switch back to the feature branch of the PR, merge the targeted branch into it, e.g. `git merge --no--ff <targeted branch name>`;
3. Push the merge to remote, and continue with the PR review/merge process.

:red_circle: Please don't use `git rebase` or `git push --force`. It will likely bring unexpected consequences.

At the end of this phase, package maintainer of a repository should create a tag on the develop branch, and update the tag collector spreadsheet.

### Phase 2 - Testing Period

In this period, changes related to the release will be made to the `prep-release/<release-name>` branch.

The start of this period is marked by the tag collection date and the build of initial candidate release. If any fixes were needed after testing the candidate release, developers will need to create a `prep-release/<release-name>` branch if it does not exist yet. This branch should be based on the initial tag for the release. The fixes can be made to the `prep-release/<release-name>` branch via pull requests with at lease one approval review.

### Phase 3 - Post Release Period

This is marked by the deployment of the release to cvmfs. No changes will be made to the deployed release, but critical bug fixes can be invited into an associated patch release. Once invited, developers should create a patch branch like `patch/dunedaq-vX.Y.x`, where `X` and `Y` denotes the MAJOR, MINOR release number, and lower-case letter `x` represents the PATCH. The patch branch should be based on the final tag used by the deployed frozen release.


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
