extern crate proc_macro;
use proc_macro::TokenStream;
use syn::{parse_macro_input, parse_quote, DataEnum, DataStruct, DeriveInput, Field, Ident, Type};
fn reflect_struct(name:Ident,data:DataStruct)->TokenStream{
    let fields:Vec<(Ident,Type)> = data.fields.iter().map(|f| {
        let name = f.ident.as_ref().unwrap();
        let ty  = f.ty.clone();
        (name.clone(), ty)
    }).collect();
    let mut out = format!("impl crate::utils::StaticReflect for {}{{fn static_reflect()->crate::utils::Type{{\n let mut fields:Vec<crate::utils::Field> = Vec::new();\n", name);
    for i in fields{
        let t = i.1;
        let t = quote::quote! {#t};
        let t = t.to_string();
        let mut tmp = String::new();
        for i in t.chars(){
            if i != ' ' && i != '<'{
                tmp.push(i);
            }
            if i == '<'{
                tmp.push(':');
                tmp.push(':');
                tmp.push('<');
            }
        }
        println!("{tmp}");
        let msg = format!(" fields.push(crate::utils::Field{{name:\"{}\".to_string(),offset: std::mem::offset_of!(Self,{}), value:{}::static_reflect()}});\n", i.0, i.0,tmp);
        out += &msg;
    }
    out += &format!("crate::utils::Type{{name:(std::any::type_name::<Self>()).to_string(), data:crate::utils::TypeData::Struct{{fields}}, size:std::mem::size_of::<Self>(), align:std::mem::align_of::<Self>()}}}} }}");
    println!("{out}");
    out.parse().unwrap()
}
fn reflect_enum(name:Ident, data:DataEnum)->TokenStream{
    todo!()
}
#[proc_macro_derive(StaticReflect)]
pub fn static_reflect_fn(item: TokenStream) -> TokenStream {
    let data = parse_macro_input!(item as DeriveInput);
    let name = data.ident;
    match data.data{
        syn::Data::Struct(data)=>{reflect_struct(name, data)},
        syn::Data::Enum(data_enum) => {reflect_enum(name,data_enum)},
        syn::Data::Union(_data_union) => {unimplemented!()},
    }
}
#[proc_macro_derive(Reflect)]
pub fn reflect_fn(item: TokenStream) -> TokenStream {
    let data = parse_macro_input!(item as DeriveInput);
    #[allow(unused)]
    let name = data.ident;
    quote::quote!(
        impl crate::utils::Reflect for #name{
            fn reflect(&self)->crate::utils::Type{
                #name::static_reflect()
            }
        }
    ).into()
}
