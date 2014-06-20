{literal}
    <SCRIPT language="javascript">
        function addRow(tableID) {
 
            var table = document.getElementById(tableID);
 
            var rowCount = table.rows.length;
            var row = table.insertRow(rowCount);
 
            var cell1 = row.insertCell(0);
            var element1 = document.createElement("input");
            element1.type = "checkbox";
            element1.name="chkbox[]";
            cell1.appendChild(element1);
 
            var cell2 = row.insertCell(1);
            cell2.innerHTML = "File "+ (rowCount-2);
 
            var cell3 = row.insertCell(2);
            var element2 = document.createElement("input");
            element2.type = "file";
            element2.name = "input-" + (rowCount-3);
            cell3.appendChild(element2);
 
//<td><input type="file" name="input-0" size="50"></td>
 
        }
 
        function deleteRow(tableID) {
            try {
            var table = document.getElementById(tableID);
            var rowCount = table.rows.length;
 
            for(var i=0; i<rowCount; i++) {
                var row = table.rows[i];
                var chkbox = row.cells[0].childNodes[0];
                if(null != chkbox && true == chkbox.checked) {
                    table.deleteRow(i);
                    rowCount--;
                    i--;
                }
 
 
            }
            }catch(e) {
                alert(e);
            }
        }
 
    </SCRIPT>
{/literal}
{include file="header.tpl"}
		<!-- main Col start-->
		<div id="hmMain">

			<h1>Figshare Uploader</h1>

			<h2>Select file to upload</h2>


<form  enctype="multipart/form-data"  METHOD="POST">

<table class="MYTABLE" id="filelist">
<tr><th></th><th>Description</th><th>File</th></tr>


<tr><td></td><td>Title</td>
<!--<td><input type="textfield" name="title" size="50"></td>-->
<td><textarea name="title" cols="50" rows="1"></textarea></td>
</tr>

<tr><td></td><td>Description</td>
<td><textarea name="description" cols="50" rows="10"></textarea></td>
</tr>

<tr><td></td><td>File 1</td>
<td><input type="file" name="file-0" size="50"></td>
</tr>

<!--<tr><td>Job description</td><td><input type="textfield" name="description" size="50"></td></tr></table>-->

</table>
<br>
<input type="button" value="Add file" onclick="addRow('filelist')"/>
<input type="button" value="Delete selected" onclick="deleteRow('filelist')"/>
</br>
</br>

{if $projects}
Select project 
<select name="project">
{html_options values=$project_idx  output=$projects selected="0"}
</select>

{else}
<p>No Projects Defined
{/if}

</br>
</br>

Select category
<select name="category">
<option value="136">NMR Spectroscopy</option>
<option value="674">Applied Chemistry</option>
<option value="700">Biochemistry</option>
<option value="731">Macromolecular Chemistry</option>
<option value="492">Physical, Inorganic and Analytical Chemstry</option>
<option value="665">Organic Chemistry</option>
<option value="71">Nuclear Chemistry</option>
<option value="75">Supramolecular Chemistry</option>
</select>
<input type="hidden" name="action" value="uploader">
<input type="hidden" name="subaction" value="upload">
<input type="hidden" name="upload" value="upload">
<br>
<br>
<input type="Submit" value="Submit" >

</form>



		</div>

{include file="footer.tpl"}
