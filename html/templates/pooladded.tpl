{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Pool Added Successfully</h1>

<table class="MYTABLE">
		<tr>	
			<th>Pool Name</th>
			<th>User</th>
			<th>Host</th>
			<th>Path</th>
			<th>Public Key</th>
			<th>Applications</th>
			<th>Status</th>
			<th>Delete</th>
			<th>Access</th>
		</tr>

	<tr>
		<td>{$pool.description}</td>
		<td>{$pool.user}</td>
		<td>{$pool.host}</td>
		<td>{$pool.path}</td>
		<td><a href="?action=pools&amp;subaction=key&amp;pool={$pool.pool_id}">Download</a></td>
		<td><a href="?action=pools&amp;subaction=list&amp;pool={$pool.pool_id}">View</a></td>
		<td><a href="?action=pools&amp;subaction=status&amp;pool={$pool.pool_id}">Status</a></td>
		<td><a href="?action=pools&amp;subaction=delete&amp;pool={$pool.pool_id}">Delete</a></td>
		<td><a href="?action=acl&amp;pool={$pool.pool_id}">Modify</a></td>
	<tr>
</table>

<p>The pool was added successfully.</p>
<p>Now
<a href="?action=pools&amp;subaction=key&amp;pool={$pool.pool_id}">download the key</a>
and add it to the ~{$pool.user}/.ssh/authorized_keys file on {$pool.host}.
Once this is done
<a href="?action=pools&amp;subaction=refresh&amp;pool={$pool.pool_id}">refresh the application list</a>.
</p>
		</div>

{include file="footer.tpl"}
