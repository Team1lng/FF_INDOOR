#!/usr/bin/env bash

set -euo pipefail

stage_all=false
push_after_commit=true
dry_run=false
yes=false
message_args=()

usage()
{
    cat <<'EOF'
Usage:
  ./daily_git_push.sh "commit message"
  ./daily_git_push.sh --no-push "commit message"
  ./daily_git_push.sh --all "commit message"
  ./daily_git_push.sh --dry-run

Options:
  --all      Stage all git changes. Use with care, generated files may be included.
  --no-push  Commit only, do not push to GitHub.
  --dry-run  Show what would be staged, but do not commit or push.
  -y, --yes  Skip confirmation prompts.
  -h, --help Show this help.

Default mode stages daily source/doc/resource changes and skips build artifacts.
EOF
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --all)
            stage_all=true
            ;;
        --no-push)
            push_after_commit=false
            ;;
        --dry-run)
            dry_run=true
            ;;
        -y|--yes)
            yes=true
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            message_args+=("$1")
            ;;
    esac
    shift
done

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

branch="$(git branch --show-current)"
if [ -z "$branch" ]; then
    echo "Error: current HEAD is detached, cannot decide push branch." >&2
    exit 1
fi

commit_msg="${message_args[*]:-daily: $(date +%Y-%m-%d) update}"

is_generated_file()
{
    case "$1" in
        */build/*|build/*|*/CMakeFiles/*|*/upgrade/platform/*|*/tools/burntool/platform/*)
            return 0
            ;;
        *.o|*.a|*.so|*.so.*|*.BIN|*.bin|*.elf|*.img|*.sqsh4|*.jffs2|*.gz|*.zip|*.rar|*.7z|*.tmp|*.log)
            return 0
            ;;
    esac
    return 1
}

is_daily_file()
{
    case "$1" in
        README.md|AGENTS.md|.gitignore|git.cmd.sh|daily_git_push.sh)
            return 0
            ;;
        *.md)
            return 0
            ;;
        FF_1070/layout/*|FF_1070/common/*|FF_1070/include/*|FF_1070/res/*|FF_1070/upgrade/*.sh|FF_1070/*.sh|FF_1070/CMakeLists.txt|FF_1070/Makefile)
            return 0
            ;;
        ff-582-70/layout/*|ff-582-70/common/*|ff-582-70/include/*|ff-582-70/res/*|ff-582-70/*.sh|ff-582-70/CMakeLists.txt|ff-582-70/Makefile)
            return 0
            ;;
        AK37E_SDK_V1.05/upgrade/*.sh|AK37E_SDK_V1.05/tools/**/*.sh)
            return 0
            ;;
    esac
    return 1
}

confirm()
{
    if [ "$yes" = true ]; then
        return 0
    fi

    printf "%s [y/N] " "$1"
    read -r answer
    case "$answer" in
        y|Y|yes|YES)
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

echo "Current branch: $branch"
echo "Commit message: $commit_msg"
echo
echo "Current status:"
git status --short
echo

if [ "$stage_all" = true ]; then
    echo "Stage mode: all changes"
    if [ "$dry_run" = false ]; then
        git add -A
    fi
else
    echo "Stage mode: safe daily files"
    staged_count=0
    skipped_count=0

    while IFS= read -r file; do
        [ -n "$file" ] || continue

        if is_generated_file "$file"; then
            printf "skip generated: %s\n" "$file"
            skipped_count=$((skipped_count + 1))
            continue
        fi

        if is_daily_file "$file"; then
            printf "stage: %s\n" "$file"
            staged_count=$((staged_count + 1))
            if [ "$dry_run" = false ]; then
                git add -- "$file"
            fi
        else
            printf "skip unknown: %s\n" "$file"
            skipped_count=$((skipped_count + 1))
        fi
    done < <(git ls-files -m -o --exclude-standard)

    echo "Selected files: $staged_count, skipped files: $skipped_count"
fi

if [ "$dry_run" = true ]; then
    echo
    echo "Dry run only. No commit or push was executed."
    exit 0
fi

echo
echo "Staged diff:"
git diff --cached --stat

if git diff --cached --quiet; then
    echo "No staged changes. Nothing to commit."
    exit 0
fi

confirm "Commit these changes?" || {
    echo "Canceled before commit."
    exit 0
}

git commit -m "$commit_msg"

if [ "$push_after_commit" = true ]; then
    confirm "Push to origin/$branch?" || {
        echo "Committed locally, push skipped."
        exit 0
    }
    git push origin "$branch"
else
    echo "Committed locally, push disabled by --no-push."
fi
