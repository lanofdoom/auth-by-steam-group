/**
 * Called if a user is not a member of the group.
 *
 * @param client_id	         The client id of the user checked.
 */
typeset AuthBySteamGroup_NotAMemberCallback {
  function void(int client_id);
};

/**
 * Verifies that the user is a member of the steam group. If not, the 
 * access denied callback is called.
 *
 * @param client_id              The client id of the user to check.
 * @param network_id	         The network id of the user to check.
 * @param steam_group_id         The Steam group to check for membership.
 * @param steam_key              The key to use for the Steam API lookups.
 * @param not_a_member_callback  Called if the user is not a member of the group.
 */
native void AuthBySteamGroup_CheckUser(
    int client_id, const char[] network_id, const char[] steam_group_id,
    const char[] steam_key,
    AuthBySteamGroup_NotAMemberCallback not_a_member_callback);