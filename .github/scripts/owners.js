module.exports = async ({ github, context, core }) => {
  const path = require("path");
  const owners = require(
    path.join(process.env.GITHUB_WORKSPACE, ".github", "owners.json"),
  );

  const pullNumber = context.payload.pull_request?.number;
  const listFiles = await github.rest.pulls.listFiles({
    owner: context.repo.owner,
    repo: context.repo.repo,
    pull_number: pullNumber,
  });

  const modifiedModules = new Set();
  for (const file of listFiles.data) {
    const filename = file.filename;
    if (filename.startsWith("modules/")) {
      modifiedModules.add(filename.split("/")[1]);
    }
  }

  if (modifiedModules.size === 0) {
    core.info("No modules were modified");
    return;
  }

  const commentParts = [
    "<!-- owners-notification-bot -->",
    "A review is required from the following owners:",
  ];
  for (const modifiedModule of modifiedModules) {
    if (owners[modifiedModule]) {
      commentParts.push(
        `- ${modifiedModule}: ${owners[modifiedModule].join(", ")}`,
      );
    }
  }

  if (commentParts.length === 2) {
    core.info("Modules were modified, but no owners were found");
    return;
  }

  const listComments = await github.rest.issues.listComments({
    owner: context.repo.owner,
    repo: context.repo.repo,
    issue_number: pullNumber,
  });
  const comment = listComments.data.find(
    (comment) =>
      comment.user.login === "github-actions[bot]" &&
      comment.body.startsWith(commentParts[0]),
  );
  if (comment) {
    core.info("Updating existing comment");
    await github.rest.issues.updateComment({
      owner: context.repo.owner,
      repo: context.repo.repo,
      comment_id: comment.id,
      body: commentParts.join("\n"),
    });
  } else {
    core.info("Creating new comment");
    await github.rest.issues.createComment({
      owner: context.repo.owner,
      repo: context.repo.repo,
      issue_number: pullNumber,
      body: commentParts.join("\n"),
    });
  }
};
