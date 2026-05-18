# Contributing

## Development Workflow

### Purpose
In order to maintain a clean and reliable production branch, all changes to `main` must be properly staged, reviewed and tested.

This section defines how branches, commits and pull requests (PRs) must be handled.

### Branching Strategy
- `main`: Production branch. Updated weekly with the latest stable version of the apps.
- `dev`: Integration branch. All features and fixes must be merged here before going to `main`.
- **Feature/Bugfix branches**: Branches off `dev` for new work. Use clear names, e.g., `add-login-ui`, `fix-payment-crash`.
- **Hotfix branches**: Only for urgent fixes that must go directly to `main`. Must follow the hotfix procedure below.

### Commits norm

Every commits must follow these rules:
1. With one or two [gitmoji](https://gitmoji.dev/), the 2nd specifying an action on the first one.
2. A short description of the change starting by an action verb at the imperative tense, try to stay under 80 characters.
3. *Optional*, if you want to provide more informations in your commit, do it in the commit's message description.

```bash title="Simple example"
📝 Update the contributing guidelines to add the commit norm.
```

```bash title="Multiple gitmojis + description"
📝🏗️ Setup production ready documentation architecture.

With 3 folders: getting started, guides & API references.
```

### Pull Request

> [!IMPORTANT]
> `dev` is the only source branch allowed for PRs targeting `main`

#### PR Title & Description
- Use a descriptive title explaining the change.
- Include a short summary, GitHub issues, List of changes/features/fixes and how you tested the feature in the description.

> [!TIP]
> Use the pull request template when creating a new one, it will set the skeleton for the description + tasks you have to do.

#### Review Requirements
At least 2 reviewers must be selected, 4 if the base branch is `main`:
- The person in charge of this part of the project.
- The Scrum Master which will check for potentially required continuous integration updates & tests to add/edit.

If new commits are pushed, all reviews will be stalled thus requiring all reviewers to do it again.

#### Check Tests
- All status checks (CI, linters, unit tests, etc) must pass before being able to merge.
- If new commits are pushed, all status checks will be relaunched.

#### Merging commits messages
The only method authorized is Squash & Merge.

The commit message must start with an appropriated [gitmoji](https://gitmoji.dev/) and resume the pull request content (you can use the title).

### Changing PR Base Branch
Avoid changing the base branch off a PR after creation.

If a PR was mistakenly created targeting `main` from another branch than `dev`, create an empty commit so the blocking CI job is retriggered and cleared.

> GitHub actually has a way to rerun workflows on PR base change but it does not dismiss old failed workflow runs.

### Hotfix Procedure
> [!WARNING]
> Hotfixes must be used only in case of an emergency production issue.
>
> Because hotfixes bypass some verification, they may require rollback or temporary disabling of affected functionality.

Branch off `main` as `hotfix/description`.

After merging back into `main`, immediately merge the hotfix into `dev` to keep branches in sync.
You must request at least 2 other people's reviews in order to merge it, ideally the same persons as in the [standard procedure](#review-requirements).
If these persons are not able to do it, you might request reviews from any two other members of the team.

### Monitoring & Compliance
- Review open PRs where you are required as reviewer regularly.

- Remember that every new commits on a PR automatically re-request a review from all reviewers.

If you find a violation of any of the rules above, mark your review as Request Change with the broken rule in the description.
This will block the PR from being merged, remember to check back more often in order to make your review again once it is fixed.

### Branch Protection Rules

#### `main` branch (production)

- Require pull requests before merging.
- Source of the PR **must be** `dev`.
- Require all status checks to pass.
- Require approval from at least 4 core team members.

#### `dev` branch

- Require pull requests before merging.
- Require all status checks to pass.
- Require at least two approval from team members, see [how to choose who](#review-requirements).
