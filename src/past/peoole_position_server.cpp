/*
入力：
tracking_id
okao_id
検出された人物属性位置

出力：
tracking_idと結びついた人物属性位置(現在／過去)
okao_idと結びついた人物属性位置(現在／過去)

処理：
いまサブスクライブしてきた人物属性情報と
むかしサブスクライブしてきた人物属性情報をいかにして判定するか

d_idをキーにして保持することのメリットって？
まあ、何番目に発見されたかを知りたいぐらい
あるいは、別の人間が同じ人間として認識されても、とりあえず情報は保持される
tracking_idじゃない理由は？順番がわかるってことかな

1.人物属性位置をサブスクライブして、d_idをキーにして保持する
2.リクエストされたtracking_idについて人物属性位置をレスポンスする
3.リクエストされたokao_idについて人物属性位置をパブリッシュする(?)

*/
