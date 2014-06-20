{include file="header.tpl"}

		<!-- main Col start-->
<div id="hmMain">

<h1>User Profile for {$profile.name}</h1>

<form>
<table>

<tr>
	<td>FOAF URL</td>
	<td><input name="foaf" size="50" value="{$profile.foaf}"></input></td>
</tr>
<tr>
	<td>ORCID ID</td>
	<td>{$profile.orcid}</td>
</tr>


<tr>
	<td>Embargo period (days)</td>
	<td>
		<input name="embargo" size="3" value="{$profile.embargo}"></input>
	</td>
</tr>
<tr>
	<td>Embargo action:</td>
	<td>
		<select name="embargoaction">
		<option value="publishandkeep">Publish and Keep</option>
		<option value="publishanddelete">Publish and Delete</option>
		<option value="delete">Delete</option>
		<option value="nothing">No action</option>
	</td>
</tr>
<!--
<tr>
/	<td>Figshare key</td>
	<td>
		<input name="figsharekey" size="80" value="{$profile.figsharekey}"></input>
	</td>
</tr>
-->
<tr>
<td>Publish to DSpace</td>
<td><input type="checkbox" name="pub_dspace" value="yes" {$profile.pub_dspace}/></td>
</tr>

<tr>
<td>Publish to Chempound</td>
<td><input type="checkbox" name="pub_chempound" value="yes" {$profile.pub_chempound}/></td>
</tr>
<tr>
<td>Publish to Figshare</td>
<td><input type="checkbox" name="pub_figshare" value="yes" {$profile.pub_figshare}/></td>
</tr>






</table>
<input type = "hidden" name ="action" value="profile" >
<input type = "hidden" name ="subaction" value="update" >
<input type="submit" value="Update">
</form>

<p><a href="figshare-auth.php">Link to Figshare</a>
<p><a href="orcid-auth.php">Link to ORCID</a>

</div>

{include file="footer.tpl"}
