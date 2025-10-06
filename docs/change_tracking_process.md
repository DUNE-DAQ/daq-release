# Documenting and tracking software changes

## Introduction

The model that we will use for documenting and tracking software changes during the `fddaq-v5.5.0` development cycle has several themes.  These are the following:

* We will make use of the tools that are available in GitHub.
* We want to have the ability to easily identify the major work areas for the release, by working group.
* We want to have the ability to easily understand the motivation, content, ramifications, and testing for each set of changes.

The GitHub tools that we will use include the following:

* the basic GitHub functionality for creating and managing Issues and Pull Requests
* "forms" that allow us to define Issue and PR templates so that users know what types of information they should provide
* the "Project" functionality that GitHub provides

GitHub Issue and PR template forms are describe [here](https://beyondco.de/blog/github-issue-forms), and our implementations of them can be seen [here](https://github.com/DUNE-DAQ/.github).  We currently support 4 different types of Issues, and we have a dedicated form for Pull Requests.

* The use of template forms is one of the ways that we hope to motivate developers and WG leaders to provide the information that will help us understand the motivation, content, ramifications, and testing for each set of changes.

GitHub Projects are described [here](https://docs.github.com/en/issues/planning-and-tracking-with-projects).  They support customizable fields that augment the information that we can use to describe a particular Issue or PR.  The Project fields that we use are described below.  Projects also support customizable views that we can use to identify issues that are planned for a particular release, group the issues by DAQ working group, etc.

* Suitably-defined Project fields and views will provide the ability to identify major deliverables for a given release along with the sub-issues and PRs that are part of them.

## Elements of the model

* The use of GitHub forms that are tailored to the type of change (e.g. bug fix or enhancement).
* The use of umbrella Issues to help make clear which Issues and PRs are part of the same deliverable.
* The use of a reasonable number of clearly-described Issue and PR status values.
* The use of a reasonable number of clearly-described Tracking Project fields.
* The introduction of a new Tracking Project field, **Impact Radius**, to provide information on the extent to which the changes will affect developers and users, will affect multiple software repositories, will change system behavior, etc.

## Project Fields for fddaq-v5.5.0

1. **Status**
    * This field indicates the state of the work.  Please see the section below for information on the supported values and their meaning.
2. **Impact Radius**
    * This field indicates how wide or large the impact of the changes will be on the system or other developers.  For example, whether or not (and how much) the software modifications
        * Include changes in multiple software repositories
        * Change software interfaces
        * Change user interfaces, including how users run the system
        * Change underlying system behavior
    * Possible values are Zero, Small/Isolated, Medium, and Large.
    * These values have the following meanings:
        * Zero:  There are effectively no code changes.  So, for example, the changes are only in documentation or the formatting of text or code.
        * Small/Isolated:  There are little or no effects outside of the repository that is being changed or the Working Group that is making the changes.
        * Medium:  this value indicates that the changes will have non-trivial, but not substantial, effects in the areas listed above, or otherwise would benefit from coordination between developers.  Developers should post a message on the _daq-release-preparation_ Slack channel when they merge correlated changes to _develop_ branches.
        * Large:  this value indicates that the changes will have substantial effects in the areas listed above, and it will be important to inform all developers of their ramifications.  Developers should post a message on the _daq-release-preparation_ Slack channel when they merge correlated changes to _develop_ branches.
    * As additional guidance, we can say that SWI&T will likely interpret these values in the following way:
        * Zero and Small/Isolated:  SWI&T does not need to be actively involved in coordinating or testing the changes.  We will trust developers and WG members to do the necessary testing, provide the necessary documentation, etc.
        * Medium:  SWI&T will expect developers to provide a target date for when the changes will be merged and will expect developers to attend SWI&T meetings to coordinate merges.  
        * Large:  SWI&T will expect developers to provide a target date for when the changes will be merged and will expect developers to attend SWI&T meetings to coordinate merges.  We will expect the developers to present information at SWI&T meetings about the impacts of the changes.  We will likely run user-acceptance-tests of the changes and review the documentation and test plans.
3. **Working Group**
    * This field indicates the DAQ Consortium Working Group that has the primary responsibility for completing the work.  Examples are CCM, Readout, and Dataflow.
4. **Target Release**
    * This field indicates the software release (stable or patch) that the work is intended to be part of.
    * Users select values from a pre-defined list.  New releases are added to the available list as they become known.  Placeholder values (e.g. fddaq-v5.x.x) are used to give general indications of intentions, where appropriate.
5. **Priority**
    * This field indicates the relative priority of this work to other work.
    * Possible values are Critical, High, Moderate, Low, and Unknown.
    * These values have the following meanings:
        * Critical:  the changes are critically needed, and a delay in completing them will likely delay the release of a new version of the software.
        * High:  the changes are very important, but they would likely not hold up a release.
        * Moderate:  the changes will be nice to have, but the timeline for including them can be flexible
        * Low:  the changes will be nice to have, but they timeline for including them can be extended to allow other work to be included beforehand
        * Unknown:  the priority of the changes has not yet been determined
6. **Target Date**
    * This field indicates the date by which the work is expected to reach the “Ready to Merge” state.  
7. **Parent Issue (for PRs)**
    * This field is used to link a child Pull Request to a parent Issue.  This is needed because GitHub currently does not provide this functionality.  The format that should be used is 'DUNE-DAQ/<repo_name>#<Issue_number>', for example, DUNE-DAQ/daq-deliverables#188.  This format helps GitHub display the text as a link, in certain cases.

## Status Field Values for fddaq-v5.5.0

1. **New** (previously Triage)
    * This status indicates that the Issue or PR has just been created, and no one has yet assigned any of the Tracking Project field values. The required elements of the GitHub Issue or PR form have been filled in, but some of them may still be works-in-progress.
2. **Todo**
    * This status indicates that someone has taken the time to set the basic Tracking Project fields and to provide meaningful content for the text in the Issue or PR. 
    * There may still be some placeholder text in the Issue or PR description, but there is enough information for a casual reader to get a sense of what the work is about.
    * The Impact Radius and Working Group fields have the correct values, at a 90% confidence level (CL).
    * The Target Release and Priority fields have values that are best guesses.
3. **Assigned to Release**
    * This status indicates that the Target Release is well-known and the text in the description text is mature.
    * The Impact Radius, Working Group, Target Release, and Priority fields all have the appropriate values set.
    * A value for the Target Date has been set.
    * Any needed parent/child links between Issues and PRs have been specified.
    * A member of the responsible Working Group should set this status.
4. **Ready for Review**
    * This status indicates that the changes to the code, documentation, tests, etc. have been completed, and the changes are ready to be reviewed.  Requested reviewer(s) have been assigned in the Issue/PR.
    * A member of the development team should set this status at the appropriate time.
    * Items should not linger in this state.
5. **Under Review**
    * This status indicates that one or more reviewers are looking at the changes.
    * One of the reviewers should set this status at the appropriate time.
    * Items should not linger in this state.
6. **Ready to Merge**
    * This status value indicates that any questions and concerns have been addressed and all of the necessary reviewers have approved the change.
    * One of the reviewers, or a member of the development team, should set this status value at the appropriate time.
7. **Done**
    * This status indicates that the work has been completed and the Issue or PR has been closed.
    * GitHub automatically sets this status value for us when the Issue or PR is closed.

A typical workflow would be the following:

* New -> Todo -> Assigned to Release -> Ready for Review -> Under Review -> Ready to Merge -> Done

However, not all status values will be necessary or useful in all cases.  For example, a given PR could skip the Todo step and could have a trivial review, in which case the Under Review status might not get set.

* New -> Assigned to Release -> Ready for Review -> Ready to Merge -> Done

The possible values for Status will typically be most useful when the Issue or PR stays in a given state for a non-trivial amount of time.  In those cases, Release Coordination and SWI&T can use the status information to ask informed questions.

For reference, we’ll say that SWI&T will likely interpret these status values in the following ways:

* **New**:  the work is not yet on the radar of SWI&T
* **Todo**:  SWI&T should take note that the described work is planned for the future
* **Assigned to Release**:  SWI&T should be paying attention to this work, including coordinating merges if needed, and requesting presentations, etc. if appropriate
* **Ready for Review**:  SWI&T (and Release Coordination) will help to make sure that Issues and PRs don’t linger in this state.
* **Under Review**:  SWI&T (and Release Coordination) will help to make sure that Issues and PRs don’t linger in this state.
* **Ready for Merge**:  SWI&T should take an active role in helping to coordinate merges, if needed.
* **Done**:  no longer needs attention

## A note regarding the Impact Radius of parent & child Issues & PRs

A natural question might be:  for a Medium impact Issue with several child Issues and/or PRs, what should the Impact Radius be for the child Issues and PRs?

Our recommendation has a couple of parts:

* The Impact Radius of a child Issue or PR can be equal-to or smaller-than the parent Issue.  For example, for a parent Issue with an Impact Radius value of Medium, child Issues and PRs can have Impact Radius values of Medium, Small/Isolated, or Zero.
* The meaning of Impact Radius for child Issues and PRs can be slightly different from parent or stand-alone Issues and PRs.  It can be used to indicate the Impact Radius of the specific change on a slightly smaller scale.  For example, a child PR that renames a source code file might be assigned a Medium Impact Radius, if the parent Issue has an Impact Radius of Medium, even though renaming a file might not qualify as a Medium change generally.  In this case, the designation of Medium would indicate that a reviewer or interested developer would notice the change in the name of the file.
