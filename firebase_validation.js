/***** ..codeauthor:: Muthukumar Subramanian *****/
/*****  HTML JavaScript to Write and Read on the Google Firebase *****/
import { initializeApp } from "https://www.gstatic.com/firebasejs/9.14.0/firebase-app.js";
import { getDatabase, ref, set, child, update, remove, onValue } from "https://www.gstatic.com/firebasejs/9.14.0/firebase-database.js";

/***** Firebase config *****/
const firebaseConfig = {
    apiKey: "AIzaSyD9AGB7gseobpK5GBhH5czsYmN8mAdcqw8",
    authDomain: "axolotl-4c586.firebaseapp.com",
    databaseURL: "https://axolotl-4c586-default-rtdb.firebaseio.com",
    projectId: "axolotl-4c586",
    storageBucket: "axolotl-4c586.appspot.com",
    messagingSenderId: "307995114526",
    appId: "1:307995114526:web:8acb93c03453521540020f"
  };

/***** Initialize Firebase *****/
const app = initializeApp(firebaseConfig);

/***** write data to firebase *****/
function write_db() {
    console.log("DEBUG: Write function");
    var db = getDatabase();
    var create_db_table = ref(db, 'ea/' + 'algo/');
    var user_name = document.getElementById("user_name").value;
    var user_password =  document.getElementById("user_name_password").value;
    if( user_name == '' || user_password == ''){
        alert("Make sure, must be non-empty data is required!!!");
        console.log("Make sure, must be non-empty data is required!!!");
        throw "Make sure, must be non-empty data is required!!!";
    }
    set(ref(db, 'ea/' + 'algo/'), {
      user_name: user_name,
      user_name_password: user_password
    }).then((res) => {
        console.log();
    })
    .catch((err) => {
        alert(err.message);
        console.log(err.code);
        console.log(err.message);
    })
}

/***** read data from firebase *****/
function read_db() {
    var db = getDatabase();
    var connect_db = ref(db, 'ea/' + 'algo/');
    var retrieve_data='';
    console.log("DEBUG: Read function");
    onValue(connect_db, (snapshot) => {
        retrieve_data = snapshot.val();
        //console.log("user_name: " + retrieve_data.user_name);
        //console.log("user_name_password: " + retrieve_data.user_name_password);
        call_loop_print(retrieve_data);
        document.getElementById("display_read_data").innerHTML =  "<pre>" + "user_name: " + retrieve_data.user_name +
                '\n' + "user_name_password: " + retrieve_data.user_name_password + "</pre>";
        })
    function call_loop_print(retrieve_data){
        for (var r=0;r<Object.entries(retrieve_data).length;r++){
            var key = Object.keys(retrieve_data)[r];
            var value = retrieve_data[key];
            console.log("Key_" + r + ': ' + key + " Value_:" + r + ': ' + value );
           }
 }
}
/***** call write data function *****/
var write_data_to_firebase = document.getElementById("write_data_to_firebase");
write_data_to_firebase.addEventListener('click', write_db);

/***** call read data function *****/
var read_data_from_firebase = document.getElementById("read_data_from_firebase");
read_data_from_firebase.addEventListener('click', read_db);