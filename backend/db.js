const mongoose = require('mongoose');
module.exports =  function dbConnection(){

    // concat uri
    const uri = `mongodb://127.0.0.1:27017`; /* /${dbConfig.dbName}`; */
    const options = {
        user: 'admin',
        pass: 'password',
        dbName: 'curtain-opener',
    }

    mongoose.connect(
        uri,
        options,
        (err) => {
            if(err){
                console.error(err);
                console.error('Db connection failure');
            } else {
                console.log('Db connected successfully');
            }
        }
    )
}