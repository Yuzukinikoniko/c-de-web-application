{
    function buttonClick(){
        var name = document.querySelector('.name').value;
        var number = document.querySelector('.number').value;
        var age = document.querySelector('.age').value;
        var data = {
            name:name,
            number:number,
            age:age
        };
        console.log(data);
        fetch('http://localhost:8000/add.html', { // 送信先URL
            method: 'POST', // 通信メソッド
            header: {
                'Content-Type': 'application/json' // JSON形式のデータのヘッダ
            },
            body: JSON.stringify(data) // JSON形式のデータ
        })
        .then(response => response.text())
        .then(data => {
            console.log(data);
        });
    }
}
