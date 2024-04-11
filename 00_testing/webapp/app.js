// Your web app's Firebase configuration (replace this with your project's config object)
var firebaseConfig = {

    apiKey: "AIzaSyBqpEv0CFXZ1mjvodzJqX1AsCsJUNAkn9Q",
    authDomain: "replenish-team16.firebaseapp.com",
    databaseURL: "https://replenish-team16-default-rtdb.firebaseio.com",
    projectId: "replenish-team16",
    storageBucket: "replenish-team16.appspot.com",
    messagingSenderId: "326923385198",
    appId: "1:326923385198:web:80e7f7b194f7f5e0fe697f"

};
  
// Initialize Firebase
firebase.initializeApp(firebaseConfig);

// Reference your database location
var database = firebase.database();

// Function to write data
function writeData1() {
    var data1 = document.getElementById('dataInput1').value;
    var intValue1 = parseInt(data1); // Make sure to convert string to integer
  
    // Using set() will overwrite data at the location
    database.ref('pump/left/').update({intValue1});
  
    // If you want to create a unique entry with an integer, use push() like this:
    // database.ref('data/').push(intValue);
    document.getElementById('dataInput1').value = ''; // Clear input after submit
}

function writeData2() {
    var data2 = document.getElementById('dataInput2').value;
    var intValue2 = parseInt(data2); // Make sure to convert string to integer
  
    // Using set() will overwrite data at the location
    database.ref('pump/middle/').update({intValue2});
  
    // If you want to create a unique entry with an integer, use push() like this:
    // database.ref('data/').push(intValue);
    document.getElementById('dataInput2').value = ''; // Clear input after submit
}
function writeData3() {
    var data3 = document.getElementById('dataInput3').value;
    var intValue3 = parseInt(data3); // Make sure to convert string to integer
  
    // Using set() will overwrite data at the location
    database.ref('pump/right/').update({intValue3});
  
    // If you want to create a unique entry with an integer, use push() like this:
    // database.ref('data/').push(intValue);
    document.getElementById('dataInput3').value = ''; // Clear input after submit
}
function writeData4() {
    var data4 = document.getElementById('dataInput4').value;
    var intValue4 = parseInt(data4); // Make sure to convert string to integer
  
    // Using set() will overwrite data at the location
    database.ref('pump/vol/').update({intValue4});
  
    // If you want to create a unique entry with an integer, use push() like this:
    // database.ref('data/').push(intValue);
    document.getElementById('dataInput4').value = ''; // Clear input after submit
}

