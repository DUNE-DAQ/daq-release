# Development workflow

We are following the [shared repository model](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/about-collaborative-development-models) and this [Git branching model](https://nvie.com/posts/a-successful-git-branching-model/) at the current stage of the DAQ software development. Under the `shared repository model`, developers are granted push access to a single shared repository. Topic branches are created when changes need to be made by following the `Git branching model`.

## Repository Access

We use [two access roles](https://home.fnal.gov/~dingpf/repo_access_role.png) for each repo.

* Maintain: one or two developers have the "Maintainer" role to each repository;
* Write: a GitHub team of developers.

Teams are entities of the DUNE-DAQ project, thus they can be used across multiple repos.

A team is usually managed by DAQ working group leaders. Developers obtain write access by being added into a team. Refer to the [List of teams and repositories](team_repos.md) page to find a list of teams and repositories each team has write access to.

## Branches of DAQ repositories

* **Long-lived branches: `develop`** (default branch of each repository);
* **Release preparation branches `prep-release/dunedaq-vX.Y.Z`** (i.e. `prep-release/dunedaq-v3.1.0`)
    * branch off from the tag created on the `develop` branch at the time of tag collection during a release cycle;
    * can be updated via PRs with at least one approval review before release cut-off time; 
    * in general, should be merged to `develop` after release cut-off by the software coordination team;
    * in case of partial merge (cherry-pick) to develop or no merge at all, developers should notify the software coordination team, and handle the partial merge by themselves. 
* **Patch branches `patch/dunedaq-vX.Y.x`** (i.e. `patch/dunedaq-v3.0.x`)
    * branch off from a tagged version used in the release where the fixes apply;
    * if the fixes apply to develop branch and should be used by the future releases, merge the patch branch to `develop` after the patch release is made;
    * in case of partial merge (cherry-pick) to develop or no merge at all, developers should notify the software coordination team, and handle the partial merge by themselves.

## Branch protection rules

* **`develop`** branch: require pull requests. All commits must be made to a non-protected branch and submitted via a pull request before they can be merged.
* **`patch/*` and `prep-release/*`** branches: in addition to requring pull requests for new commits, the pull request must have at least one approval review before they can be merged.

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

We have adopted a three-phased release cycle:
1. Phase-1, active development period;
2. Phase-2, testing period;
3. Phase-3, post release (patch release) period.

### Phase 1 - Active Development Period
 
In this period, developers make frequent updates to the `develop` branch via pull requests. The workflow will be like the following:

1. Create a GitHub issue in the repo describe the code change. This is optional for small changes.
2. Create a topic branch; (`git checkout develop; git checkout -b dingpf/issue_12_feature_dev_demo`)
3. Make code development, commit, and push the topic branch to GitHub; (`git push -u origin dingpf/issue_12_feature_dev_demo`)
4. Create a pull request to the `develop` branch and link the issue to the pull request if one was created in step 1;
5. Technically, the pull request can be merged without reviews. But it's highly recommended the author request reviews from other developers if the code change is significant.

The active development period comes to an end when the develop branch is ready to be tagged. The procedure for this is described in the next section. It is _highly_ recommended that before this is done the package's codebase is checked for:

1. [compliance with our coding guidelines](https://dune-daq-sw.readthedocs.io/en/latest/packages/styleguide/) -- in particular that `dbt-build` is run with the `--lint` option and no major issues revealed
2. `dbt-clang-format.sh` is run on the codebase so that whitespace formatting is correct
3. If your package is a dependency of another package, a correct set `find_dependency` calls in `cmake/<packagename>Config.cmake.in`. It's often the case that developers update dependencies in `CMakeLists.txt` without making the corresponding update(s) in `cmake/<packagename>Config.cmake.in`.

Details on the first two steps above can be found in the [daq-buildtools documentation](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-buildtools/#useful-build-options). Details on the third step can be found in the [daq-cmake documentation](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#installing-your-project-as-a-local-package).


### Phase 2 - Testing Period

#### Before the testing period starts

Developers need to bump the version of the package on the develop branch. Either on or before the tag collection date, the person in charge of tagging the package (typically the package maintainer, or whoever is marked as such on the tag collector spreadsheet) should do the following:
1. Consult the tag collector spreadsheet to confirm they're assigned as the package tagger, and to confirm the new version number `<X.Y.Z>`. Any disagreement or confusion about either of these should be resolved before the next step. The spreadsheet is by convention linked to [from the top of the "Instructions for setting up a development area" page of the daqconf Wiki](https://github.com/DUNE-DAQ/daqconf/wiki/Instructions-for-setting-up-a-development-software-area)
2. Update the `project(<package name> VERSION <X.Y.Z>)` line at the top of `CMakeLists.txt`, and go through a trivial PR if the `develop` branch hasn't yet had its protection rule removed by the software coordination team for the release process.
3. With the `CMakeLists.txt` modification committed on `develop`, perform an annotated tag on `develop`: `git tag -a v<X.Y.Z> -m "<your initials>: version v<X.Y.Z>"`
4. Push your `develop` branch and your tag to the central repo: `git push origin develop; git push --tags`
5. Mark your package as "Tag Ready" on the tag collector spreadsheet

#### During the testing period

An initial candidate release will be built once the first round of tags are collected. That marks the start of the testing period. Any further changes made during the testing period should be agreed upon and significant - this is not a time for introducing minor new features, as we want to test as consistent a codebase as possible. *
1. Changes which do get made will be made to a `prep-release/dunedaq-v<X.Y.Z>` branch; if one doesn't exist, it should be created;
2. This branch should be based on the initial tag for the release; 
3. The fixes can be made to the `prep-release/dunedaq-v<X.Y.Z>` branch via pull requests with at lease one approval review.

### Phase 3 - Post Release Period

This is marked by the deployment of the release to cvmfs. No changes will be made to the deployed release, but critical bug fixes can be invited into an associated patch release. Once invited, developers should create a patch branch like `patch/dunedaq-vX.Y.x`, where `X` and `Y` denotes the MAJOR, MINOR release number, and lower-case letter `x` represents the PATCH. The patch branch should be based on the final tag used by the deployed frozen release. When the code on the patch branch is ready, the package maintainer should make a version tag for the patch release off the patch branch. 



## Useful tips


💡 If the targeted branch of a pull request has advanced, please do the following to bring the feature branch in sync before merging the PR:
1. Switch to the targeted branch, and do a `git pull` to make sure it stays in sync with the remote;
2. Switch back to the feature branch of the PR, merge the targeted branch into it, e.g. `git merge --no--ff <targeted branch name>`;
3. Push the merge to remote, and continue with the PR review/merge process.

:red_circle: Please don't use `git rebase` or `git push --force`. It will likely bring unexpected consequences.

* Using `#Issue_Number` in your commit message will make GitHub add links to the commit on the issue page;
* Use `user/repo#issue_number` to link issues in a different repo, e.g. `DUNE-DAQ/daq-cmake#1`;
* **Use with caution** Delete a git tag: `git tag -d v1.0.1` **Do not delete any tag which might be used by others**;
* Push the deletion of a tag to GitHub: `git push --delete origin v1.0.1` **Do not delete any tag which might be used by others**;
* Delete a remote branch: `git push origin --delete feature/branch_name` (not recommended; recommend to delete branches via GitHub web UI);
* List all tags in GitHub repo: `git ls-remote --tags origin`;
* Fetch all branches and tags: `git fetch --all --tags`.

<!---
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
--->
