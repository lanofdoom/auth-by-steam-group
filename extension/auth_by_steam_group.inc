/**
 * Synchronously checks if a user is a member of the indicated steam group.
 * TODO: Rewrite this as async
 *
 * @param client_id              The client id of the user to check.
 * @param network_id             The network id of the user to check.
 * @param steam_group_id         The Steam group to check for membership.
 * @param steam_key              The key to use for the Steam API lookups.
 */
native bool AuthBySteamGroup_CheckUser(
    int client_id, const char[] network_id, const char[] steam_group_id,
    const char[] steam_key);