
{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Manage Users</h1>


	<form>
	<table class="MYTABLE">
	<tr><th>User</th><th>Admin</th><th>Blocked</th></tr>
	{section name=sec1 loop=$manage}
		<tr>
		<td>{$manage[sec1].uname}</td>
		{if $manage[sec1].admin=="1"}
		<td align=center><input type="checkbox" name="ADM{$manage[sec1].user_id}" checked="1"></td>
		{else}
		<td align=center><input type="checkbox" name="ADM{$manage[sec1].user_id}"></td>
		{/if}
		{if $manage[sec1].blocked=="1"}
		<td align=center><input type="checkbox" name="BLK{$manage[sec1].user_id}" checked="1"></td>
		{else}
		<td align=center><input type="checkbox" name="BLK{$manage[sec1].user_id}"></td>
		{/if}
		</tr>
	{/section}
	</table>
	<input type="hidden" name="action" value="manage">
	<input type="hidden" name="subaction" value="set">
	<input type="submit" value="Update">
	</form>




		</div>

{include file="footer.tpl"}
