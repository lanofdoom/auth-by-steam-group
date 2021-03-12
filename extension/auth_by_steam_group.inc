/**
 * Allows the next unauthenticated user to join the server if the user
 * requesting the authentication bypass is a member of the stream group.
 * Success and failure are reported asynchronously.
 *
 * @param client_id              The client id requesting the auth bypass.
 * @param steam_group_id         The Steam group to check for membership.
 * @param steam_key              The key to use for the Steam API lookups.
 */
native void AuthBySteamGroup_AllowJoin(
    int client_id, const char[] steam_group_id, const char[] steam_key);

/**
 * Disables allows access allowed by AuthBySteamGroup_AllowJoin if active.
 *
 * @param client_id              The client id requesting the auth bypass.
 * @param steam_group_id         The Steam group to check for membership.
 * @param steam_key              The key to use for the Steam API lookups.
 */
native void AuthBySteamGroup_DenyJoin(
    int client_id, const char[] steam_group_id, const char[] steam_key);

/**
 * Checks if a user is a member of the indicated steam group. If the check
 * fails, the user will be kicked asynchronously.
 *
 * @param client_id              The client id of the user to check.
 * @param steam_group_id         The Steam group to check for membership.
 * @param steam_key              The key to use for the Steam API lookups.
 */
native void AuthBySteamGroup_CheckUser(
    int client_id, const char[] steam_group_id, const char[] steam_key);

/**
 * Should be called once per frame to check for completed asynchronous work.
 * This is a bit of a hack, but it requires less code than hooking into the
 * frame callback directly from the extension code.
 */
native void AuthBySteamGroup_OnFrame();

/**
 * Checks if a user is a member of the indicated group. If so, prints the list
 * of kickable users to them. Otherwise, the command prints an error.
 *
 * @param client_id              The client id of the user to check.
 * @param steam_group_id         The Steam group to check for membership.
 * @param steam_key              The key to use for the Steam API lookups.
 */
native void AuthBySteamGroup_PrintKickList(
    int client_id, const char[] steam_group_id, const char[] steam_key);

/**
 * Attempt to process a command from a user as a kick command. Returns true if
 * the command matched a kick command. Results of a matched command are
 * returned asynchronously.
 *
 * @param client_id              The client id of the user to check.
 * @param steam_group_id         The Steam group to check for membership.
 * @param steam_key              The key to use for the Steam API lookups.
 * @param text                   The command sent by the user.
 */
native bool AuthBySteamGroup_TryProcessKickCommand(
    int client_id, const char[] steam_group_id, const char[] steam_key,
    const char[] text);

/**
 * Rotates the list of users granted access to the server. Users allowed access
 * to the server without group credentials will be allowed to migrate from one
 * rotation to the next as long as they remain connected to the server. Should
 * a full rotation elapse in which those users did not connect, their temporary
 * access will expire.
 */
native void AuthBySteamGroup_RotateAccessGrants();