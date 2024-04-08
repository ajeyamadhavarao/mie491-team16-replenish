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
function writeData() {
    var data = document.getElementById('dataInput').value;
    var intValue = parseInt(data); // Make sure to convert string to integer
  
    // Using set() will overwrite data at the location
    database.ref('pump/left/').update({intValue});
  
    // If you want to create a unique entry with an integer, use push() like this:
    // database.ref('data/').push(intValue);
    document.getElementById('dataInput').value = ''; // Clear input after submit
}

// Function to read data
database.ref('pump/left/').on('value', function(snapshot) {
var display = document.getElementById('dataDisplay');
display.innerHTML = ''; // Clear previous data
snapshot.forEach(function(childSnapshot) {
    var childData = childSnapshot.val();
    display.innerHTML += childData.content + '<br>';
});
});
