{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Compute Pools</h1>

	<h2>My Pools</h2>

{if $my_pools}	
<table class="MYTABLE">
		<tr>	
			<th>Pool Name</th>
			<th>Host</th>
			<th>User</th>
			<th>Script Path</th>
			<th>Public Key</th>
			<th>Applications</th>
			<th>Status</th>
			<th>Delete</th>
			<th>Access</th>
		</tr>

{section name=sec1 loop=$pools}
	{if $pools[sec1].mine}
	<tr>
		<td>{$pools[sec1].description}</td>
		<td>{$pools[sec1].host}</td>
		<td>{$pools[sec1].user}</td>
		<td>{$pools[sec1].path}</td>
		<td><a href="?action=pools&subaction=key&pool={$pools[sec1].pool_id}">Download</a></td>
		<td><a href="?action=pools&subaction=list&pool={$pools[sec1].pool_id}">View</a></td>
		<td><a href="?action=pools&subaction=status&pool={$pools[sec1].pool_id}">Status</a></td>
		<td><a href="?action=pools&subaction=delete&pool={$pools[sec1].pool_id}">Delete</a></td>
		<td><a href="?action=acl&pool={$pools[sec1].pool_id}">Modify</a></td>
	<tr>
	{/if}
{/section}
</table>
{else}
	<p>No pools
{/if}

<H2> Other Pools</H2>

{if $other_pools}	
<table class="MYTABLE">
		<tr>	
			<th>Pool Name</th>
			<th>Host</th>
			<th>Applications</th>
			<th>Status</th>
		</tr>

{section name=sec1 loop=$pools}
	{if !$pools[sec1].mine}
	<tr>
		<td>{$pools[sec1].description}</td>
		<td>{$pools[sec1].host}</td>
		<td><a href="?action=pools&subaction=list&pool={$pools[sec1].pool_id}">View</a></td>
		<td><a href="?action=pools&subaction=status&pool={$pools[sec1].pool_id}">Status</a></td>
	<tr>
	{/if}
{/section}
</table>
{else}
	<p>No pools
{/if}


<h2>Add Pool</h2>
<p>To add a new pool, fill in the form and click the &quot;Add&quot; button.</p>
<form method="post">
	<table class="MYTABLE">
	<tr><td>Pool Name</td>
	<td><Input name ="description" type="textfield"></input></td>
    <td>The name of the new pool</td>
	</tr>
	<tr><td>Host</td>
	<td><Input name ="host" type="textfield"></input></td>
    <td>The host machine portal will run the jobs on.</td>
	</tr>
	<tr><td>User</td>
	<td><Input name ="user" type="textfield"></input></td>
    <td>The username the portal will use to connect to the host.</td>
	</tr>
	<tr><td>Script Path</td>
	<td><Input name ="path" type="textfield"></input></td>
    <td>The path to the scripts the portal will use to run the jobs.</td>
	</tr>
	<input type = "hidden" name ="action" value="pools" >
	<input type = "hidden" name ="subaction" value="add" >
	</table>
    <p></p>
	<input type="submit" value="Add">
</form>
</table>





		</div>

{include file="footer.tpl"}
