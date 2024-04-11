// Your web app's Firebase configuration
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

// Reference to your database
var database = firebase.database();

// Function to handle the toggle switch change
function handleToggle(toggleId, path) {
  var checkbox = document.getElementById(toggleId);
  var slider = checkbox.nextElementSibling; // Assuming slider is right after checkbox

  if (checkbox.checked) {
    // Add 'on' class to make it visibly turned on
    slider.classList.add('on');
    // Update the database path with value 1 to indicate on state
    database.ref(path).update({value: 1});
  } else {
    // This part will execute if the checkbox is clicked when already checked
    slider.classList.remove('on');
    // Update the database with value 0 to indicate off state
    database.ref(path).update({value: 0});
  }
}

// Function to dispense and reset all toggles
function writeData4() {
  var volumeInput = document.getElementById('dataInput4');
  var value = parseInt(volumeInput.value);

  database.ref('pump/vol/').update({value});
  volumeInput.value = '';

  // Reset all toggles and remove 'on' class to revert to the normal state
  ['toggle1', 'toggle2', 'toggle3'].forEach(function(toggleId) {
    var checkbox = document.getElementById(toggleId);
    var slider = checkbox.nextElementSibling;
    checkbox.checked = false;
    slider.classList.remove('on');
  });
}

// Event listeners for the toggle switches
document.addEventListener('DOMContentLoaded', function() {
  document.getElementById('toggle1').addEventListener('change', function() {
    handleToggle('toggle1', 'pump/left/');
  });
  document.getElementById('toggle2').addEventListener('change', function() {
    handleToggle('toggle2', 'pump/middle/');
  });
  document.getElementById('toggle3').addEventListener('change', function() {
    handleToggle('toggle3', 'pump/right/');
  });
  
  // Attach event listener to the dispense button to reset toggles after dispensing
  document.getElementById('dispenseButton').addEventListener('click', writeData4);
});
