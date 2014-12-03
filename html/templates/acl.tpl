{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Access Control List for Pool {$poolname}</h1>


	<form>
	<table class="MYTABLE">
	<tr><th>User</th><th>Access</th></tr>
	{section name=sec1 loop=$acl}
		<tr>
		<td>{$acl[sec1].uname}</td>
		{if $acl[sec1].member=="1"}
		<td align=center><input type="checkbox" name="ACL{$acl[sec1].user_id}" checked="1"></td>
		{else}
		<td align=center><input type="checkbox" name="ACL{$acl[sec1].user_id}"></td>
		{/if}
		</tr>
	{/section}
	</table>
    <p>
	<input type="hidden" name="action" value="acl">
	<input type="hidden" name="subaction" value="set">
	<input type="hidden" name="pool"   value="{$pool}">
    {if $is_admin==1}
    <h2>Admin option</h2>
        {if $is_public==1}
        <p>Make this pool public: <input type="checkbox" name="public" checked="1"></p>
        {else}
        <p>Make this pool public: <input type="checkbox" name="public"></p>
        {/if}

    {/if}
	<input type="submit" value="Update">
	</form>




		</div>

{include file="footer.tpl"}
