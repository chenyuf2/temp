//click the button and update the txt file
function clickbuttonfunc() {
  var userinput = document.getElementById("searchfield").value; //It gives the input of user, written in form of string.
  var splitted_userinput = userinput.split(','); // we split the user input and we want to write it to a new file called topics_dm.txt
  const fs = require('fs') 
  
// Data which will write in a file. 
let data = "Learning how to write in a file."
  
// Write data in 'Output.txt' . 
fs.writeFile('Output.txt', data, (err) => { 
      
    // In case of a error throw err. 
    if (err) throw err; 
}) 
  
  
}