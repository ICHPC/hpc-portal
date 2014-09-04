{include file="header.tpl"}

		<!-- main Col start-->
		<div id="hmMain">

			<h1>Please log in below</h1>


			<p>
	{if $error}
	{$error}
	{/if}

<form name='logindata' method='post'>
<input type="hidden" name="action" value="login">
<table width='100%' border='0'>
<tr>
<td align='right'>
User Name: 
</td>
<td align='left'>
<INPUT TYPE='text' NAME='username'>
</td>
</tr>
<tr>

<td align='right'>
Password: 
</td>
<td align='left'>
<INPUT TYPE='password' NAME='password'>
</td>
</tr>
<tr>
<td>
</td>
<td align='left'>
<INPUT TYPE='submit' VALUE='Login'>

		
	</td>

</tr>
</table>
</form>
			</p>




		</div>

{include file="footer.tpl"}
